// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "mobipocket.h"
#include "decompressor.h"

#include <QBuffer>
#include <QIODevice>
#include <QImageReader>
#include <QRegularExpression>
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif
#include <QtEndian>

namespace Mobipocket
{

struct PDBPrivate {
    QList<quint32> recordOffsets;
    QIODevice *device;
    QString fileType;
    quint16 nrecords;
    bool valid;

    void init();
};

void PDBPrivate::init()
{
    valid = true;
    quint16 word;
    quint32 dword;
    if (!device->seek(0x3c))
        goto fail;
    fileType = QString::fromLatin1(device->read(8));

    if (!device->seek(0x4c))
        goto fail;
    device->read((char *)&word, 2);
    nrecords = qFromBigEndian(word);

    for (int i = 0; i < nrecords; i++) {
        device->read((char *)&dword, 4);
        recordOffsets.append(qFromBigEndian(dword));
        device->read((char *)&dword, 4);
    }
    return;
fail:
    valid = false;
}

PDB::PDB(QIODevice *device)
    : d(new PDBPrivate)
{
    d->device = device;
    d->init();
}

PDB::~PDB()
{
    delete d;
}

QByteArray PDB::getRecord(int i) const
{
    if (i >= d->nrecords)
        return QByteArray();
    quint32 offset = d->recordOffsets[i];
    bool last = (i == (d->nrecords - 1));
    if (!d->device->seek(offset))
        return QByteArray();
    if (last)
        return d->device->readAll();
    return d->device->read(d->recordOffsets[i + 1] - offset);
}

bool PDB::isValid() const
{
    return d->valid;
}

int PDB::recordCount() const
{
    return d->nrecords;
}

struct DocumentPrivate 
{
    DocumentPrivate(QIODevice *d)
        : pdb(d)
    {
    }
    PDB pdb;
    std::unique_ptr<Decompressor> dec;
    quint16 ntextrecords = 0;
    quint16 maxRecordSize = 0;
    bool valid = false;

    // number of first record holding image. Usually it is directly after end of text, but not always
    quint16 firstImageRecord = 0;
    QMap<Document::MetaKey, QString> metadata;
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder toUtf16;
#else
    QTextCodec *codec = nullptr;
#endif
    bool drm = false;
    quint32 extraflags = 0;

    // index of thumbnail in image list. May be specified in EXTH.
    // If not then just use first image and hope for the best
    int thumbnailIndex = 0;

    void init();
    void findFirstImage();
    void parseEXTH(const QByteArray &data);
    void parseHtmlHead(const QString &data);
    QString readStringRecord(const QByteArray &data);
};

void DocumentPrivate::parseHtmlHead(const QString &data)
{
    static const QRegularExpression title(QLatin1String("<dc:title.*>(.*)</dc:title>"),
                                          QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    static const QRegularExpression author(QLatin1String("<dc:creator.*>(.*)</dc:creator>"),
                                           QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    static const QRegularExpression copyright(QLatin1String("<dc:rights.*>(.*)</dc:rights>"),
                                              QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    static const QRegularExpression subject(QLatin1String("<dc:subject.*>(.*)</dc:subject>"),
                                            QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    static const QRegularExpression description(QLatin1String("<dc:description.*>(.*)</dc:description>"),
                                                QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);

    // title could have been already taken from MOBI record
    if (!metadata.contains(Document::Title)) {
        if (const auto titleMatch = title.match(data); titleMatch.hasMatch())
            metadata[Document::Title] = titleMatch.captured(1);
    }
    if (const auto authorMatch = author.match(data); authorMatch.hasMatch())
        metadata[Document::Author] = authorMatch.captured(1);
    if (const auto copyrightMatch = copyright.match(data); copyrightMatch.hasMatch())
        metadata[Document::Copyright] = copyrightMatch.captured(1);
    if (const auto subjectMatch = subject.match(data); subjectMatch.hasMatch())
        metadata[Document::Subject] = subjectMatch.captured(1);
    if (const auto descriptionMatch = description.match(data); descriptionMatch.hasMatch())
        metadata[Document::Description] = descriptionMatch.captured(1);
}

namespace {
    const QVector<QByteArray> getHuffRecords(const PDB &pdb)
    {
        const QByteArray header = pdb.getRecord(0);
        if (header[1] != 'H') {
            return {};
        }

        quint32 huff_ofs = qFromBigEndian<quint32>(header.constData() + 0x70);
        quint32 huff_num = qFromBigEndian<quint32>(header.constData() + 0x74);

        QVector<QByteArray> records(huff_num);
        for (quint32 i = 0; i < huff_num; i++) {
            if (auto r = pdb.getRecord(huff_ofs + i); r.isNull()) {
                return {};
            } else {
                records[i] = r;
            }
        }
        return records;
    };
}

void DocumentPrivate::init()
{
    quint32 encoding = 0;

    if (!pdb.isValid())
        return;
    QByteArray mhead = pdb.getRecord(0);
    if (mhead.isNull() || mhead.size() < 14)
        return;

    dec = Decompressor::create(mhead[1], getHuffRecords(pdb));
    if ((int)mhead[12] != 0 || (int)mhead[13] != 0)
        drm = true;
    if (!dec)
        return;

    ntextrecords = qFromBigEndian<quint16>(mhead.constData() + 8);
    maxRecordSize = qFromBigEndian<quint16>(mhead.constData() + 10);
    if (mhead.size() > 31)
        encoding = qFromBigEndian<quint32>(mhead.constData() + 28);
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    if (encoding == 65001) {
        toUtf16 = QStringDecoder(QStringDecoder::Utf8);
    } else {
        const auto converterEncoding = QStringConverter::encodingForName("cp1252");
        if (converterEncoding) {
            toUtf16 = QStringDecoder(*converterEncoding);
        } else {
            toUtf16 = QStringDecoder(QStringConverter::Latin1);
        }
    }
#else
    if (encoding == 65001)
        codec = QTextCodec::codecForName("UTF-8");
    else
        codec = QTextCodec::codecForName("CP1252");
#endif
    if (mhead.size() >= 92)
        parseEXTH(mhead);

    quint32 exthoffs = qFromBigEndian<quint32>(mhead.constData() + 20);
    if ((mhead.size() >= 244) && ((exthoffs + 16) > 244)) {
        extraflags = qFromBigEndian<quint32>(mhead.constData() + 240);
    }

    // try getting metadata from HTML if nothing or only title was recovered from MOBI and EXTH records
    if (metadata.size() < 2 && !drm)
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        parseHtmlHead(toUtf16(dec->decompress(pdb.getRecord(1))));
#else
        parseHtmlHead(codec->toUnicode(dec->decompress(pdb.getRecord(1))));
#endif
    valid = true;
}

void DocumentPrivate::findFirstImage()
{
    firstImageRecord = ntextrecords + 1;
    while (firstImageRecord < pdb.recordCount()) {
        QByteArray rec = pdb.getRecord(firstImageRecord);
        if (rec.isNull())
            return;
        QBuffer buf(&rec);
        buf.open(QIODevice::ReadOnly);
        QImageReader r(&buf);
        if (r.canRead())
            return;
        firstImageRecord++;
    }
}

QString DocumentPrivate::readStringRecord(const QByteArray &data)
{
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    return toUtf16(data);
#else
    return codec->toUnicode(data);
#endif
}

void DocumentPrivate::parseEXTH(const QByteArray &data)
{
    // try to get name
    if (data.size() >= 92) {
        qint32 nameoffset = qFromBigEndian<quint32>(data.constData() + 84);
        qint32 namelen = qFromBigEndian<quint32>(data.constData() + 88);
        if ((nameoffset + namelen) <= data.size()) {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
            metadata[Document::Title] = toUtf16(data.mid(nameoffset, namelen));
#else
            metadata[Document::Title] = codec->toUnicode(data.mid(nameoffset, namelen));
#endif
        }
    }

    quint32 exthoffs = qFromBigEndian<quint32>(data.constData() + 20);

    if (data.mid(exthoffs + 16, 4) != "EXTH")
        return;
    quint32 records = qFromBigEndian<quint32>(data.constData() + exthoffs + 24);
    quint32 offset = exthoffs + 28;
    for (unsigned int i = 0; i < records; i++) {
        if (offset + 8 > quint32(data.size()))
            break;
        quint32 type = qFromBigEndian<quint32>(data.constData() + offset);
        quint32 len = qFromBigEndian<quint32>(data.constData() + offset + 4);
        if (offset + len > quint32(data.size()))
            break;
        switch (type) {
        case 100:
            metadata[Document::Author] = readStringRecord(data.mid(offset + 8, len - 8));
            break;
        case 103:
            metadata[Document::Description] = readStringRecord(data.mid(offset + 8, len - 8));
            break;
        case 105:
            metadata[Document::Subject] = readStringRecord(data.mid(offset + 8, len - 8));
            break;
        case 109:
            metadata[Document::Copyright] = readStringRecord(data.mid(offset + 8, len - 8));
            break;
        case 202:
            thumbnailIndex = qFromBigEndian<quint32>(data.constData() + offset + 8);
            break;
        default:
            // ignore
            break;
        }
        offset += len;
    }
}

Document::Document(QIODevice *dev)
    : d(new DocumentPrivate(dev))
{
    Q_ASSERT(dev->openMode() & QIODevice::ReadOnly);
    Q_ASSERT(!dev->isSequential());
    d->init();
}

Document::~Document()
{
    delete d;
}

namespace {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
constexpr qsizetype preTrailingDataLength(QByteArrayView data, quint32 flags)
#else
qsizetype preTrailingDataLength(QByteArray data, quint32 flags)
#endif
{
    if (flags == 0) {
        return data.size();
    }

    for (int i = 31; i > 0; i--) {
        if ((flags & (1u << i)) == 0) {
            continue;
        }

        qsizetype chopN = 0;
        for (int j = 0; j < 4; j++) {
            if (j + 1 > data.size()) {
                return 0;
            }
            quint8 l = data.at(data.size() - (j + 1));
            chopN |= (l & 0x7f) << (7 * j);
            if (l & 0x80) {
                break;
            }
        }
        data.chop(std::min<qsizetype>(chopN, data.size()));
    }
    if ((flags & 0x1) && !data.isEmpty()) {
        quint8 l = data.back() & 0x3;
        data.chop(std::min<qsizetype>(l + 1, data.size()));
    }
    return data.size();
}
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
static_assert(preTrailingDataLength({"0\x00", 2}, 0x0) == 2);
static_assert(preTrailingDataLength({"0\x00", 2}, 0x1) == 1);
static_assert(preTrailingDataLength({"0\x01", 2}, 0x1) == 0);
static_assert(preTrailingDataLength({"0\x02", 2}, 0x1) == 0);
static_assert(preTrailingDataLength({"abcd\x03", 5}, 0x1) == 1);
static_assert(preTrailingDataLength({"abcd\x81", 5}, 0x2) == 4);
static_assert(preTrailingDataLength({"\x02\x01", 2}, 0x2) == 0);
static_assert(preTrailingDataLength({"\x80\x02", 2}, 0x2) == 0);
static_assert(preTrailingDataLength({"abcd\x85", 5}, 0x2) == 0);
static_assert(preTrailingDataLength({"abc\x01\x7f\x82", 6}, 0x2) == 4);
static_assert(preTrailingDataLength({"abc\x01\x80\x02", 6}, 0x2) == 4);
static_assert(preTrailingDataLength({"abc\x01\x7f\x82", 6}, 0x3) == 2);
static_assert(preTrailingDataLength({"abc\x81\x80\x02", 6}, 0x6) == 3);
static_assert(preTrailingDataLength({"abc\x00\x81\x81", 6}, 0x7) == 3);
#endif
} // namespace

QString Document::text(int size) const
{
    QByteArray whole;
    for (int i = 1; i < d->ntextrecords + 1; i++) {
        auto record = d->pdb.getRecord(i);
        record.resize(preTrailingDataLength(record, d->extraflags));
        QByteArray decompressedRecord = d->dec->decompress(record);
        whole += decompressedRecord;
        if (!d->dec->isValid()) {
            d->valid = false;
            return QString();
        }
        if (size != -1 && whole.size() > size)
            break;
    }
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    return d->toUtf16(whole);
#else
    return d->codec->toUnicode(whole);
#endif
}

int Document::imageCount() const
{
    // FIXME: don't count FLIS and FCIS records
    return d->pdb.recordCount() - d->ntextrecords;
}

bool Document::isValid() const
{
    return d->valid;
}

QImage Document::getImage(int i) const
{
    if (!d->firstImageRecord)
        d->findFirstImage();

    if ((i < 0) || (i > std::numeric_limits<quint16>::max()) //
        || (d->firstImageRecord + i) >= d->pdb.recordCount()) {
        return {};
    }

    QByteArray rec = d->pdb.getRecord(d->firstImageRecord + i);
    return (rec.isNull()) ? QImage() : QImage::fromData(rec);
}

QMap<Document::MetaKey, QString> Document::metadata() const
{
    return d->metadata;
}

bool Document::hasDRM() const
{
    return d->drm;
}

QImage Document::thumbnail() const
{
    QImage img = getImage(d->thumbnailIndex);

    // does not work, try first image
    if (img.isNull() && d->thumbnailIndex) {
        d->thumbnailIndex = 0;
        img = getImage(0);
    }
    return img;
}

}
