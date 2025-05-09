// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "mobipocket.h"
#include "decompressor.h"

#include <QBuffer>
#include <QIODevice>
#include <QImageReader>
#include <QRegularExpression>
#include <QTextCodec>
#include <QtEndian>

namespace Mobipocket
{

QByteArray Stream::read(int len)
{
    QByteArray ret;
    ret.resize(len);
    len = read(ret.data(), len);
    if (len < 0)
        len = 0;
    ret.resize(len);
    return ret;
}

QByteArray Stream::readAll()
{
    QByteArray ret, bit;
    while (!(bit = read(4096)).isEmpty())
        ret += bit;
    return ret;
}

struct PDBPrivate {
    QList<quint32> recordOffsets;
    Stream *device;
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

PDB::PDB(Stream *dev)
    : d(new PDBPrivate)
{
    d->device = dev;
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

////////////////////////////////////////////
struct DocumentPrivate {
    DocumentPrivate(Stream *d)
        : pdb(d)
        , valid(true)
        , firstImageRecord(0)
        , drm(false)
        , thumbnailIndex(0)
    {
    }
    PDB pdb;
    std::unique_ptr<Decompressor> dec;
    quint16 ntextrecords;
    quint16 maxRecordSize;
    bool valid;

    // number of first record holding image. Usually it is directly after end of text, but not always
    quint16 firstImageRecord;
    QMap<Document::MetaKey, QString> metadata;
    QTextCodec *codec;
    bool drm;

    // index of thumbnail in image list. May be specified in EXTH.
    // If not then just use first image and hope for the best
    int thumbnailIndex;

    void init();
    void findFirstImage();
    void parseEXTH(const QByteArray &data);
    void parseHtmlHead(const QString &data);
    QString readEXTHRecord(const QByteArray &data, quint32 &offset);
    QImage getImageFromRecord(int recnum);
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

    valid = pdb.isValid();
    if (!valid)
        return;
    QByteArray mhead = pdb.getRecord(0);
    if (mhead.isNull() || mhead.size() < 14)
        goto fail;
    dec = Decompressor::create(mhead[1], pdb);
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
    if (encoding == 65001)
        codec = QTextCodec::codecForName("UTF-8");
    else
        codec = QTextCodec::codecForName("CP1252");
    if (mhead.size() > 176)
        parseEXTH(mhead);

    // try getting metadata from HTML if nothing or only title was recovered from MOBI and EXTH records
    if (metadata.size() < 2 && !drm)
        parseHtmlHead(codec->toUnicode(dec->decompress(pdb.getRecord(1))));
    return;
fail:
    valid = false;
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

QString DocumentPrivate::readEXTHRecord(const QByteArray &data, quint32 &offset)
{
    quint32 len = readBELong(data, offset);
    offset += 4;
    len -= 8;
    QString ret = codec->toUnicode(data.mid(offset, len));
    offset += len;
    return ret;
}

QImage DocumentPrivate::getImageFromRecord(int i)
{
    QByteArray rec = pdb.getRecord(i);
    return (rec.isNull()) ? QImage() : QImage::fromData(rec);
}

void DocumentPrivate::parseEXTH(const QByteArray &data)
{
    // try to get name
    if (data.size() >= 92) {
        qint32 nameoffset = readBELong(data, 84);
        qint32 namelen = readBELong(data, 88);
        if ((nameoffset + namelen) < data.size()) {
            metadata[Document::Title] = codec->toUnicode(data.mid(nameoffset, namelen));
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

Document::Document(Stream *dev)
    : d(new DocumentPrivate(dev))
{
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
        QByteArray decompressedRecord = d->dec->decompress(d->pdb.getRecord(i));
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
    return d->codec->toUnicode(whole);
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
    return d->getImageFromRecord(d->firstImageRecord + i);
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
    QImage img = d->getImageFromRecord(d->thumbnailIndex + d->firstImageRecord);
    // does not work, try first image
    if (img.isNull() && d->thumbnailIndex) {
        d->thumbnailIndex = 0;
        img = d->getImageFromRecord(d->firstImageRecord);
    }
    return img;
}

}
