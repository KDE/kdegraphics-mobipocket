// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "mobipocket.h"
#include "decompressor.h"
#include "kpdb.h"

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

struct DocumentPrivate 
{
    DocumentPrivate(QIODevice *d)
        : pdbFile()
        , valid(true)
        , firstImageRecord(0)
        , drm(false)
        , thumbnailIndex(0)
    {
        pdbFile.read(d);
    }
    KPDBFile pdbFile;
    std::unique_ptr<Decompressor> dec;
    quint16 ntextrecords;
    quint16 maxRecordSize;
    bool valid;

    // number of first record holding image. Usually it is directly after end of text, but not always
    quint16 firstImageRecord;
    QMap<Document::MetaKey, QString> metadata;
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder toUtf16;
#else
    QTextCodec *codec = nullptr;
#endif
    bool drm;

    // index of thumbnail in image list. May be specified in EXTH.
    // If not then just use first image and hope for the best
    int thumbnailIndex;

    void init();
    void findFirstImage();
    void parseEXTH(const QByteArray &data);
    void parseHtmlHead(const QString &data);
    QString readEXTHRecord(const QByteArray &data, quint32 &offset);
    QImage imageFromRecordAt(int recnum) const;
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

void DocumentPrivate::init()
{
    quint32 encoding = 0;

    valid = pdbFile.isValid();
    if (!valid)
        return;
    QByteArray mhead = pdbFile.header().name();
    if (mhead.isNull() || mhead.size() < 14)
        goto fail;
    dec = Decompressor::create(mhead[1], pdbFile);
    if ((int)mhead[12] != 0 || (int)mhead[13] != 0)
        drm = true;
    if (!dec)
        goto fail;

    ntextrecords = (unsigned char)mhead[8];
    ntextrecords <<= 8;
    ntextrecords += (unsigned char)mhead[9];
    maxRecordSize = (unsigned char)mhead[10];
    maxRecordSize <<= 8;
    maxRecordSize += (unsigned char)mhead[11];
    if (mhead.size() > 31)
        encoding = readBELong(mhead, 28);
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
    if (mhead.size() > 176)
        parseEXTH(mhead);

    // try getting metadata from HTML if nothing or only title was recovered from MOBI and EXTH records
    if (metadata.size() < 2 && !drm)
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        parseHtmlHead(toUtf16(dec->decompress(pdbFile.recordAt(1))));
#else
        parseHtmlHead(codec->toUnicode(dec->decompress(pdbFile.recordAt(1))));
#endif
    return;
fail:
    valid = false;
}

void DocumentPrivate::findFirstImage()
{
    firstImageRecord = ntextrecords + 1;
    while (firstImageRecord < pdbFile.header().recordCount()) {
        auto rec = pdbFile.recordAt(firstImageRecord);
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

QString DocumentPrivate::readEXTHRecord(const QByteArray &data, quint32 &offset)
{
    quint32 len = readBELong(data, offset);
    offset += 4;
    len -= 8;
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    QString ret = toUtf16(data.mid(offset, len));
#else
    QString ret = codec->toUnicode(data.mid(offset, len));
#endif
    offset += len;
    return ret;
}

QImage DocumentPrivate::imageFromRecordAt(int i) const
{
    const auto rec = pdbFile.recordAt(i);
    return (rec.isNull()) ? QImage() : QImage::fromData(rec);
}

void DocumentPrivate::parseEXTH(const QByteArray &data)
{
    // try to get name
    if (data.size() >= 92) {
        qint32 nameoffset = readBELong(data, 84);
        qint32 namelen = readBELong(data, 88);
        if ((nameoffset + namelen) < data.size()) {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
            metadata[Document::Title] = toUtf16(data.mid(nameoffset, namelen));
#else
            metadata[Document::Title] = codec->toUnicode(data.mid(nameoffset, namelen));
#endif
        }
    }

    quint32 exthoffs = readBELong(data, 20) + 16;

    if (data.mid(exthoffs, 4) != "EXTH")
        return;
    quint32 records = readBELong(data, exthoffs + 8);
    quint32 offset = exthoffs + 12;
    for (unsigned int i = 0; i < records; i++) {
        if (offset + 4 > quint32(data.size()))
            break;
        quint32 type = readBELong(data, offset);
        offset += 4;
        switch (type) {
        case 100:
            metadata[Document::Author] = readEXTHRecord(data, offset);
            break;
        case 103:
            metadata[Document::Description] = readEXTHRecord(data, offset);
            break;
        case 105:
            metadata[Document::Subject] = readEXTHRecord(data, offset);
            break;
        case 109:
            metadata[Document::Copyright] = readEXTHRecord(data, offset);
            break;
        case 202:
            offset += 4;
            thumbnailIndex = readBELong(data, offset);
            offset += 4;
            break;
        default:
            readEXTHRecord(data, offset);
        }
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

QString Document::text(int size) const
{
    QByteArray whole;
    for (int i = 1; i < d->ntextrecords + 1; i++) {
        QByteArray decompressedRecord = d->dec->decompress(d->pdbFile.recordAt(i));
        if (decompressedRecord.size() > d->maxRecordSize)
            decompressedRecord.resize(d->maxRecordSize);
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
    return d->pdbFile.header().recordCount() - d->ntextrecords;
}

bool Document::isValid() const
{
    return d->valid;
}

QImage Document::getImage(int i) const
{
    if (!d->firstImageRecord)
        d->findFirstImage();
    return d->imageFromRecordAt(d->firstImageRecord + i);
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
    if (!d->firstImageRecord)
        d->findFirstImage();
    QImage img = d->imageFromRecordAt(d->thumbnailIndex + d->firstImageRecord);
    // does not work, try first image
    if (img.isNull() && d->thumbnailIndex) {
        d->thumbnailIndex = 0;
        img = d->imageFromRecordAt(d->firstImageRecord);
    }
    return img;
}

}
