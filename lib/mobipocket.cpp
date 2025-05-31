// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "mobipocket.h"
#include "decompressor.h"
#include "kpdb_p.h"
#include "mobiheader_p.h"
#include "palmdocheader_p.h"

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

template<typename T>
T safeRead(const QByteArray &data, quint32 offset)
{
    if (data.size() < offset + static_cast<quint32>(sizeof(T))) {
        return 0;
    }
    return qFromBigEndian<T>(data.constData() + offset);
}

namespace
{

constexpr auto MOBI_HEADER_V7_SIZE = 0xe4;
constexpr uint MOBI_TITLE_SIZEMAX = 1024;

enum Type {
    Numeric,
    String,
    DateTime,
    Binary,
};

struct ExthMetadata {
    Mobipocket::Document::MetaKey metaKey;
    Type type;
    QLatin1String description; // TODO translation?
};

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
constexpr auto static exthMetadata = std::to_array<ExthMetadata>({
#else
static std::vector<ExthMetadata> exthMetadata = {
#endif
    ExthMetadata{Mobipocket::Document::Sample, Numeric, QLatin1String("Sample")},
        ExthMetadata{Mobipocket::Document::StartThreading, Numeric, QLatin1String("Start offset")},
        ExthMetadata{Mobipocket::Document::KF8Boundary, Numeric, QLatin1String("K8 Boundary Offset")},
        ExthMetadata{Mobipocket::Document::CountResources, Numeric, QLatin1String("K8 Resources Count")}, // of , fonts, images
        ExthMetadata{Mobipocket::Document::RESCOffset, Numeric, QLatin1String("RESC Offset")},
        ExthMetadata{Mobipocket::Document::CoverOffset, Numeric, QLatin1String("Cover Offset")},
        ExthMetadata{Mobipocket::Document::ThumbnailOffset, Numeric, QLatin1String("Thumbnail Offset")},
        ExthMetadata{Mobipocket::Document::HasFakeCover, Numeric, QLatin1String("Has Fake Cover")},
        ExthMetadata{Mobipocket::Document::CreatorSoftware, Numeric, QLatin1String("Creator Software")},
        ExthMetadata{Mobipocket::Document::CreatorMajorVersion, Numeric, QLatin1String("Creator Major Version")},
        ExthMetadata{Mobipocket::Document::CreatorMinorVersion, Numeric, QLatin1String("Creator Minor Version")},
        ExthMetadata{Mobipocket::Document::CreatorBuild, Numeric, QLatin1String("Creator Build Number")},
        ExthMetadata{Mobipocket::Document::ClippingLimit, Numeric, QLatin1String("Clipping Limit")},
        ExthMetadata{Mobipocket::Document::PublisherLimit, Numeric, QLatin1String("Publisher Limit")},
        ExthMetadata{Mobipocket::Document::TTSDisable, Numeric, QLatin1String("Text-to-Speech Disabled")},
        ExthMetadata{Mobipocket::Document::Rental, Numeric, QLatin1String("Rental Indicator")},
        ExthMetadata{Mobipocket::Document::DrmServer, String, QLatin1String("DRM Server ID")},
        ExthMetadata{Mobipocket::Document::DrmCommerce, String, QLatin1String("DRM Commerce ID")},
        ExthMetadata{Mobipocket::Document::DrmBookbase, String, QLatin1String("DRM Ebookbase Book ID")},
        ExthMetadata{Mobipocket::Document::Title, String, QLatin1String("Title")}, ExthMetadata{Mobipocket::Document::Author, String, QLatin1String("Creator")},
        ExthMetadata{Mobipocket::Document::Publisher, String, QLatin1String("Publisher")},
        ExthMetadata{Mobipocket::Document::Imprint, String, QLatin1String("Imprint")},
        ExthMetadata{Mobipocket::Document::Description, String, QLatin1String("Description")},
        ExthMetadata{Mobipocket::Document::ISBN, String, QLatin1String("ISBN")}, ExthMetadata{Mobipocket::Document::Subject, String, QLatin1String("Subject")},
        ExthMetadata{Mobipocket::Document::PublishingDate, DateTime, QLatin1String("Published")},
        ExthMetadata{Mobipocket::Document::Review, String, QLatin1String("Review")},
        ExthMetadata{Mobipocket::Document::Contributor, String, QLatin1String("Contributor")},
        ExthMetadata{Mobipocket::Document::Rights, String, QLatin1String("Rights")},
        ExthMetadata{Mobipocket::Document::SubjectCode, String, QLatin1String("Subject Code")},
        ExthMetadata{Mobipocket::Document::Type, String, QLatin1String("Type")}, ExthMetadata{Mobipocket::Document::Source, String, QLatin1String("Source")},
        ExthMetadata{Mobipocket::Document::ASIN, String, QLatin1String("ASIN")},
        ExthMetadata{Mobipocket::Document::Version, String, QLatin1String("Version Number")},
        ExthMetadata{Mobipocket::Document::Adult, String, QLatin1String("Adult")}, ExthMetadata{Mobipocket::Document::Price, String, QLatin1String("Price")},
        ExthMetadata{Mobipocket::Document::Currency, String, QLatin1String("Currency")},
        ExthMetadata{Mobipocket::Document::FixedLayout, String, QLatin1String("Fixed Layout")},
        ExthMetadata{Mobipocket::Document::BookType, String, QLatin1String("Book Type")},
        ExthMetadata{Mobipocket::Document::OrientationLock, String, QLatin1String("Orientation Lock")},
        ExthMetadata{Mobipocket::Document::OriginalResolution, String, QLatin1String("Original Resolution")},
        ExthMetadata{Mobipocket::Document::ZeroGutter, String, QLatin1String("Zero Gutter")},
        ExthMetadata{Mobipocket::Document::ZeroMargin, String, QLatin1String("Zero margin")},
        ExthMetadata{Mobipocket::Document::KF8CoverUri, String, QLatin1String("K8 Masthead/Cover Image")},
        ExthMetadata{Mobipocket::Document::RegionMag, String, QLatin1String("Region Magnification")},
        ExthMetadata{Mobipocket::Document::DictionaryName, String, QLatin1String("Dictionary Short Name")},
        ExthMetadata{Mobipocket::Document::Watermark, String, QLatin1String("Watermark")},
        ExthMetadata{Mobipocket::Document::Doctype, String, QLatin1String("Document Type")},
        ExthMetadata{Mobipocket::Document::LastUpdate, String, QLatin1String("Last Update Time")},
        ExthMetadata{Mobipocket::Document::UpdatedTitle, String, QLatin1String("Updated Title")},
        ExthMetadata{Mobipocket::Document::ASIN504, String, QLatin1String("ASIN (504)")},
        ExthMetadata{Mobipocket::Document::TitleFileAs, String, QLatin1String("Title File As")},
        ExthMetadata{Mobipocket::Document::CreatorFileAs, String, QLatin1String("Creator File As")},
        ExthMetadata{Mobipocket::Document::PublisherFileAs, String, QLatin1String("Publisher File As")},
        ExthMetadata{Mobipocket::Document::Language, String, QLatin1String("Language")},
        ExthMetadata{Mobipocket::Document::Alignment, String, QLatin1String("Primary Writing Mode")},
        ExthMetadata{Mobipocket::Document::PageDir, String, QLatin1String("Page Progression Direction")},
        ExthMetadata{Mobipocket::Document::OverrideKindleFonts, String, QLatin1String("Override Kindle Fonts")},
        ExthMetadata{Mobipocket::Document::OriginalSourceDescription, String, QLatin1String("Original Source description")},
        ExthMetadata{Mobipocket::Document::DictionaryInputLanguage, String, QLatin1String("Dictionary Input Language")},
        ExthMetadata{Mobipocket::Document::DictionaryOutputLanguage, String, QLatin1String("Dictionary Output Language")},
        ExthMetadata{Mobipocket::Document::InputSource, String, QLatin1String("Input Source")},
        ExthMetadata{Mobipocket::Document::CreatorBuildRevision, String, QLatin1String("Kindlegen BuildRev Number")},
        ExthMetadata{Mobipocket::Document::TamperKeys, Binary, QLatin1String("Tamper Proof Keys")},
        ExthMetadata{Mobipocket::Document::FontSignature, Binary, QLatin1String("Font Signature")},
        ExthMetadata{Mobipocket::Document::ReadForFree, Binary, QLatin1String("Read For Free")},
        ExthMetadata{Mobipocket::Document::Unknown403, Binary, QLatin1String("Unknown (403)")},
        ExthMetadata{Mobipocket::Document::Unknown407, Binary, QLatin1String("Unknown (407)")},
        ExthMetadata{Mobipocket::Document::Unknown450, Binary, QLatin1String("Unknown (450)")},
        ExthMetadata{Mobipocket::Document::Unknown451, Binary, QLatin1String("Unknown (451)")},
        ExthMetadata{Mobipocket::Document::Unknown452, Binary, QLatin1String("Unknown (452)")},
        ExthMetadata{Mobipocket::Document::Unknown453, Binary, QLatin1String("Unknown (453)")},
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
});
#else
};
#endif
}

namespace Mobipocket
{

struct DocumentPrivate 
{
    DocumentPrivate(QIODevice *d)
        : pdbFile(*d)
    {
    }
    KPDBFile pdbFile;
    PalmDocHeader palmDocHeader;
    MobiHeader mobiHeader;
    std::unique_ptr<Decompressor> dec;
    bool valid = false;
    bool isKF8 = false;

    // number of first record holding image. Usually it is directly after end of text, but not always
    quint16 firstImageRecord = 0;
    QMap<Document::MetaKey, QVariant> metadata;
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder toUtf16;
#else
    QTextCodec *codec = nullptr;
#endif
    bool drm = false;

    // index of thumbnail in image list. May be specified in EXTH.
    // If not then just use first image and hope for the best
    int thumbnailIndex = 0;

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

namespace {
    const QVector<QByteArray> getHuffRecords(const KPDBFile &pdb)
    {
        const QByteArray header = pdb.recordAt(0);
        if (header[1] != 'H') {
            return {};
        }

        quint32 huff_ofs = qFromBigEndian<quint32>(header.constData() + 0x70);
        quint32 huff_num = qFromBigEndian<quint32>(header.constData() + 0x74);

        QVector<QByteArray> records(huff_num);
        for (quint32 i = 0; i < huff_num; i++) {
            if (auto r = pdb.recordAt(huff_ofs + i); r.isNull()) {
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
    if (!pdbFile.isValid())
        return;

    if (pdbFile.header().databaseType() != "TEXt" && pdbFile.header().databaseType() != "BOOK") {
        qWarning() << "Unsupported file";
        valid = false;
        return;
    }

    // Parse PalmDoc Header
    const QByteArray mhead = pdbFile.recordAt(0);
    if (mhead.isNull() || mhead.size() < 16) {
        qWarning() << "Empty record0 in mobipocket file";
        valid = false;
        return;
    }

    palmDocHeader.compression = qFromBigEndian<quint16>(mhead.constData());
    palmDocHeader.textLength = qFromBigEndian<quint32>(mhead.constData() + 4);
    palmDocHeader.recordCount = qFromBigEndian<quint16>(mhead.constData() + 8);
    palmDocHeader.recordSize = qFromBigEndian<quint16>(mhead.constData() + 10);
    palmDocHeader.encryptionType = qFromBigEndian<quint16>(mhead.constData() + 12);

    dec = Decompressor::create(palmDocHeader.compression, getHuffRecords(pdbFile));
    if (!dec)
        return;

    drm = palmDocHeader.encryptionType != 0;

    // Parse Mobi Header
    if (mhead.size() <= 20) {
        valid = false;
        return;
    }
    mobiHeader.mobiMagic = mhead.mid(16, 4);
    if (mobiHeader.mobiMagic != "MOBI") {
        valid = false;
        return;
    }

    mobiHeader.headerLength = safeRead<quint32>(mhead, 20);
    quint32 mobiType = safeRead<quint32>(mhead, 24);
    mobiHeader.mobiType = static_cast<MobiHeader::MobiType>(mobiType);
    mobiHeader.textEncoding = safeRead<quint32>(mhead, 28);

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    if (mobiHeader.textEncoding == 0 || mobiHeader.textEncoding == 65001) {
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
    if (mobiHeader.textEncoding == 0 || mobiHeader.textEncoding == 65001)
        codec = QTextCodec::codecForName("UTF-8");
    else
        codec = QTextCodec::codecForName("CP1252");
#endif

    mobiHeader.uid = safeRead<quint32>(mhead, 32);
    mobiHeader.version = safeRead<quint32>(mhead, 36);

    if (mobiHeader.headerLength >= MOBI_HEADER_V7_SIZE && mobiHeader.version == 8) {
        isKF8 = true;
    }

    mobiHeader.orthIndex = safeRead<quint32>(mhead, 40);
    mobiHeader.inflIndex = safeRead<quint32>(mhead, 44);
    mobiHeader.namesIndex = safeRead<quint32>(mhead, 48);
    mobiHeader.keysIndex = safeRead<quint32>(mhead, 52);
    mobiHeader.extra0Index = safeRead<quint32>(mhead, 56);
    mobiHeader.extra1Index = safeRead<quint32>(mhead, 60);
    mobiHeader.extra2Index = safeRead<quint32>(mhead, 64);
    mobiHeader.extra3Index = safeRead<quint32>(mhead, 68);
    mobiHeader.extra4Index = safeRead<quint32>(mhead, 72);
    mobiHeader.extra5Index = safeRead<quint32>(mhead, 76);
    mobiHeader.nonTextIndex = safeRead<quint32>(mhead, 80);
    mobiHeader.fullNameOffset = safeRead<quint32>(mhead, 84);
    mobiHeader.fullNameLength = safeRead<quint32>(mhead, 88);
    mobiHeader.locale = safeRead<quint32>(mhead, 92);
    mobiHeader.dictInputLang = safeRead<quint32>(mhead, 96);
    mobiHeader.dictOutputLang = safeRead<quint32>(mhead, 100);
    mobiHeader.minVersion = safeRead<quint32>(mhead, 104);
    mobiHeader.imageIndex = safeRead<quint32>(mhead, 108);
    mobiHeader.huffRecIndex = safeRead<quint32>(mhead, 112);
    mobiHeader.huffRecCount = safeRead<quint32>(mhead, 116);
    mobiHeader.datpRecIndex = safeRead<quint32>(mhead, 120);
    mobiHeader.datpRecCount = safeRead<quint32>(mhead, 124);
    mobiHeader.exthFlags = safeRead<quint32>(mhead, 128);

    // 32 unknown bytes

    mobiHeader.unknown6 = safeRead<quint32>(mhead, 164);
    mobiHeader.drmOffset = safeRead<quint32>(mhead, 168);
    mobiHeader.drmCount = safeRead<quint32>(mhead, 172);
    mobiHeader.drmSize = safeRead<quint32>(mhead, 176);
    mobiHeader.drmFlags = safeRead<quint32>(mhead, 180);

    // 8 unknown bytes

    if (isKF8) {
        mobiHeader.fdstIndex = safeRead<quint32>(mhead, 192);
    } else {
        mobiHeader.firstTextIndex = safeRead<quint16>(mhead, 192);
        mobiHeader.lastTextIndex = safeRead<quint16>(mhead, 194);
    }
    mobiHeader.fdstSectionCount = safeRead<quint32>(mhead, 196);
    mobiHeader.fcisIndex = safeRead<quint32>(mhead, 200);
    mobiHeader.fcisCount = safeRead<quint32>(mhead, 204);
    mobiHeader.flisIndex = safeRead<quint32>(mhead, 208);
    mobiHeader.flisCount = safeRead<quint32>(mhead, 212);
    mobiHeader.unknown10 = safeRead<quint32>(mhead, 216);
    mobiHeader.unknown11 = safeRead<quint32>(mhead, 220);
    mobiHeader.srcsIndex = safeRead<quint32>(mhead, 224);
    mobiHeader.srcsCount = safeRead<quint32>(mhead, 228);
    mobiHeader.unknown12 = safeRead<quint32>(mhead, 232);
    mobiHeader.unknown13 = safeRead<quint32>(mhead, 236);

    // skip 2 bytes

    mobiHeader.extraFlags = safeRead<quint16>(mhead, 242);
    mobiHeader.ncxIndex = safeRead<quint32>(mhead, 244);
    if (isKF8) {
        mobiHeader.fragmentIndex = safeRead<quint32>(mhead, 248);
        mobiHeader.skeletonIndex = safeRead<quint32>(mhead, 252);
    } else {
        mobiHeader.unknown14 = safeRead<quint32>(mhead, 248);
        mobiHeader.unknown15 = safeRead<quint32>(mhead, 252);
    }
    mobiHeader.datpIndex = safeRead<quint32>(mhead, 256);
    if (isKF8) {
        mobiHeader.guideIndex = safeRead<quint32>(mhead, 260);
    } else {
        mobiHeader.unknown16 = safeRead<quint32>(mhead, 260);
    }

    mobiHeader.unknown17 = safeRead<quint32>(mhead, 264);
    mobiHeader.unknown18 = safeRead<quint32>(mhead, 268);
    mobiHeader.unknown19 = safeRead<quint32>(mhead, 272);
    mobiHeader.unknown20 = safeRead<quint32>(mhead, 276);

    // try to get name
    if (mobiHeader.fullNameOffset && mobiHeader.fullNameLength) {
        const quint32 fullNameLength = std::min(mobiHeader.fullNameLength, MOBI_TITLE_SIZEMAX);
        if ((mobiHeader.fullNameOffset + fullNameLength) <= mhead.size()) {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
            metadata[Document::Title] = QString(toUtf16(mhead.mid(mobiHeader.fullNameOffset, mobiHeader.fullNameLength)));
#else
            metadata[Document::Title] = codec->toUnicode(mhead.mid(mobiHeader.fullNameOffset, mobiHeader.fullNameLength));
#endif
        }
    }

    if (mobiHeader.exthFlags & 0x40) {
        parseEXTH(mhead);
    }

    // try getting metadata from HTML if nothing or only title was recovered from MOBI and EXTH records
    if (metadata.size() < 2 && !drm) {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        parseHtmlHead(toUtf16(dec->decompress(pdbFile.recordAt(1))));
#else
        parseHtmlHead(codec->toUnicode(dec->decompress(pdbFile.recordAt(1))));
#endif
    }
    valid = true;
}

void DocumentPrivate::findFirstImage()
{
    if (mobiHeader.imageIndex) {
        firstImageRecord = mobiHeader.imageIndex;
    } else {
        firstImageRecord = palmDocHeader.recordCount + 1;
    }
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
    quint32 len = qFromBigEndian<quint32>(data.constData() + offset);
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
    quint32 exthoffs = qFromBigEndian<quint32>(data.constData() + 20);

    if (data.mid(exthoffs + 16, 4) != "EXTH")
        return;
    quint32 records = qFromBigEndian<quint32>(data.constData() + exthoffs + 24);
    quint32 offset = exthoffs + 28;
    for (unsigned int i = 0; i < records; i++) {
        if (offset + 4 > quint32(data.size()))
            break;
        auto type = static_cast<Document::MetaKey>(qFromBigEndian<quint32>(data.constData() + offset));
        offset += 4;

        auto it = std::find_if(exthMetadata.cbegin(), exthMetadata.cend(), [type](auto metadata) {
            return metadata.metaKey == type;
        });

        if (it == exthMetadata.cend()) {
            // Unknown key
            readEXTHRecord(data, offset);
            continue;
        }

        if (it->type == String) {
            metadata[it->metaKey] = readEXTHRecord(data, offset);
        } else {
            quint32 len = qFromBigEndian<quint32>(data.constData() + offset);
            offset += 4;
            len -= 8;
            const auto byteArray = data.mid(offset, len);

            if (it->type == Numeric) {
                metadata[it->metaKey] =
                    (quint16)((quint16)byteArray[0] << 24 | (quint16)byteArray[1] << 16 | (quint16)byteArray[2] << 8 | (quint16)byteArray[3]);
            } else if (it->type == DateTime) {
                const auto date = QString::fromUtf8(byteArray);
                metadata[it->metaKey] = QDateTime::fromString(date, Qt::ISODate);
                if (!metadata[it->metaKey].isValid()) {
                    metadata[it->metaKey] =
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
                        QDateTime::fromString(date.first(date.indexOf(QStringLiteral(".")) + 4) + date.mid(date.indexOf(QStringLiteral(".")) + 7),
                                              QStringLiteral("yyyy-MM-dd HH:mm:ss.zzzttt"));
#else
                        QDateTime::fromString(date.left(date.indexOf(QStringLiteral(".")) + 4) + date.mid(date.indexOf(QStringLiteral(".")) + 7),
                                              QStringLiteral("yyyy-MM-dd HH:mm:ss.zzzttt"));
#endif
                }
            } else if (it->type == Binary) {
                metadata[it->metaKey] = QString::fromUtf8(byteArray);
            }
            offset += len;
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
    for (int i = 1; i < d->palmDocHeader.recordCount + 1; i++) {
        QByteArray decompressedRecord = d->dec->decompress(d->pdbFile.recordAt(i));
        if (decompressedRecord.size() > d->palmDocHeader.recordSize)
            decompressedRecord.resize(d->palmDocHeader.recordSize);
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
    return d->pdbFile.header().recordCount() - d->palmDocHeader.recordCount;
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

QMap<Document::MetaKey, QVariant> Document::metadata() const
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
    const int thumbnailIndex = d->metadata[ThumbnailOffset].toInt();
    QImage img = d->imageFromRecordAt(thumbnailIndex + d->firstImageRecord);
    // does not work, try first image
    if (img.isNull() && thumbnailIndex) {
        img = d->imageFromRecordAt(d->firstImageRecord);
    }
    return img;
}

const MobiHeader &Document::mobiHeader() const
{
    return d->mobiHeader;
}

QString Document::formatMetadata(Document::MetaKey metaKey, const QVariant &metadata) const
{
    auto it = std::find_if(exthMetadata.cbegin(), exthMetadata.cend(), [metaKey](auto metadata) {
        return metadata.metaKey == metaKey;
    });

    if (it == exthMetadata.cend()) {
        return {};
    }

    if (metadata.userType() == QMetaType::QString) {
        return QStringLiteral("%1: %2").arg(it->description, metadata.toString());
    } else if (metadata.userType() == QMetaType::Int) {
        return QStringLiteral("%1: %2").arg(it->description, QString::number(metadata.toInt()));
    } else if (metadata.userType() == QMetaType::QDateTime) {
        return QStringLiteral("%1: %2").arg(it->description, metadata.toDateTime().toString());
    } else {
        return QStringLiteral("%1: %2").arg(it->description, QStringLiteral("Unknown type"));
    }
}

bool Document::isKF8() const
{
    return d->mobiHeader.headerLength >= MOBI_HEADER_V7_SIZE && d->mobiHeader.version == 8;
}

QString Document::plainText() const
{
    const auto input = text();

    QString output;
    output.reserve(input.size());

    bool insideTag = false;
    for (QChar ch : input) {
        if (ch == u'<') {
            insideTag = true;
            continue;
        }
        if (insideTag) {
            if (ch == u'>') {
                insideTag = false;
            }
            continue;
        }
        if (ch == u'\t') {
            continue;
        }
        output.append(ch);
    }

    return output;
}
}
