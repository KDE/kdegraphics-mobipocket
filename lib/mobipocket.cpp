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

constexpr auto static exthMetadata = std::to_array<ExthMetadata>({
    {Mobipocket::Document::Sample, Numeric, QLatin1String("Sample")},
    {Mobipocket::Document::StartThreading, Numeric, QLatin1String("Start offset")},
    {Mobipocket::Document::KF8Boundary, Numeric, QLatin1String("K8 Boundary Offset")},
    {Mobipocket::Document::CountResources, Numeric, QLatin1String("K8 Resources Count")}, // of , fonts, images
    {Mobipocket::Document::RESCOffset, Numeric, QLatin1String("RESC Offset")},
    {Mobipocket::Document::CoverOffset, Numeric, QLatin1String("Cover Offset")},
    {Mobipocket::Document::ThumbnailOffset, Numeric, QLatin1String("Thumbnail Offset")},
    {Mobipocket::Document::HasFakeCover, Numeric, QLatin1String("Has Fake Cover")},
    {Mobipocket::Document::CreatorSoftware, Numeric, QLatin1String("Creator Software")},
    {Mobipocket::Document::CreatorMajorVersion, Numeric, QLatin1String("Creator Major Version")},
    {Mobipocket::Document::CreatorMinorVersion, Numeric, QLatin1String("Creator Minor Version")},
    {Mobipocket::Document::CreatorBuild, Numeric, QLatin1String("Creator Build Number")},
    {Mobipocket::Document::ClippingLimit, Numeric, QLatin1String("Clipping Limit")},
    {Mobipocket::Document::PublisherLimit, Numeric, QLatin1String("Publisher Limit")},
    {Mobipocket::Document::TTSDisable, Numeric, QLatin1String("Text-to-Speech Disabled")},
    {Mobipocket::Document::Rental, Numeric, QLatin1String("Rental Indicator")},
    {Mobipocket::Document::DrmServer, String, QLatin1String("DRM Server ID")},
    {Mobipocket::Document::DrmCommerce, String, QLatin1String("DRM Commerce ID")},
    {Mobipocket::Document::DrmBookbase, String, QLatin1String("DRM Ebookbase Book ID")},
    {Mobipocket::Document::Title, String, QLatin1String("Title")},
    {Mobipocket::Document::Author, String, QLatin1String("Creator")},
    {Mobipocket::Document::Publisher, String, QLatin1String("Publisher")},
    {Mobipocket::Document::Imprint, String, QLatin1String("Imprint")},
    {Mobipocket::Document::Description, String, QLatin1String("Description")},
    {Mobipocket::Document::ISBN, String, QLatin1String("ISBN")},
    {Mobipocket::Document::Subject, String, QLatin1String("Subject")},
    {Mobipocket::Document::PublishingDate, DateTime, QLatin1String("Published")},
    {Mobipocket::Document::Review, String, QLatin1String("Review")},
    {Mobipocket::Document::Contributor, String, QLatin1String("Contributor")},
    {Mobipocket::Document::Rights, String, QLatin1String("Rights")},
    {Mobipocket::Document::SubjectCode, String, QLatin1String("Subject Code")},
    {Mobipocket::Document::Type, String, QLatin1String("Type")},
    {Mobipocket::Document::Source, String, QLatin1String("Source")},
    {Mobipocket::Document::ASIN, String, QLatin1String("ASIN")},
    {Mobipocket::Document::Version, String, QLatin1String("Version Number")},
    {Mobipocket::Document::Adult, String, QLatin1String("Adult")},
    {Mobipocket::Document::Price, String, QLatin1String("Price")},
    {Mobipocket::Document::Currency, String, QLatin1String("Currency")},
    {Mobipocket::Document::FixedLayout, String, QLatin1String("Fixed Layout")},
    {Mobipocket::Document::BookType, String, QLatin1String("Book Type")},
    {Mobipocket::Document::OrientationLock, String, QLatin1String("Orientation Lock")},
    {Mobipocket::Document::OriginalResolution, String, QLatin1String("Original Resolution")},
    {Mobipocket::Document::ZeroGutter, String, QLatin1String("Zero Gutter")},
    {Mobipocket::Document::ZeroMargin, String, QLatin1String("Zero margin")},
    {Mobipocket::Document::KF8CoverUri, String, QLatin1String("K8 Masthead/Cover Image")},
    {Mobipocket::Document::RegionMag, String, QLatin1String("Region Magnification")},
    {Mobipocket::Document::DictionaryName, String, QLatin1String("Dictionary Short Name")},
    {Mobipocket::Document::Watermark, String, QLatin1String("Watermark")},
    {Mobipocket::Document::Doctype, String, QLatin1String("Document Type")},
    {Mobipocket::Document::LastUpdate, String, QLatin1String("Last Update Time")},
    {Mobipocket::Document::UpdatedTitle, String, QLatin1String("Updated Title")},
    {Mobipocket::Document::ASIN504, String, QLatin1String("ASIN (504)")},
    {Mobipocket::Document::TitleFileAs, String, QLatin1String("Title File As")},
    {Mobipocket::Document::CreatorFileAs, String, QLatin1String("Creator File As")},
    {Mobipocket::Document::PublisherFileAs, String, QLatin1String("Publisher File As")},
    {Mobipocket::Document::Language, String, QLatin1String("Language")},
    {Mobipocket::Document::Alignment, String, QLatin1String("Primary Writing Mode")},
    {Mobipocket::Document::PageDir, String, QLatin1String("Page Progression Direction")},
    {Mobipocket::Document::OverrideKindleFonts, String, QLatin1String("Override Kindle Fonts")},
    {Mobipocket::Document::OriginalSourceDescription, String, QLatin1String("Original Source description")},
    {Mobipocket::Document::DictionaryInputLanguage, String, QLatin1String("Dictionary Input Language")},
    {Mobipocket::Document::DictionaryOutputLanguage, String, QLatin1String("Dictionary Output Language")},
    {Mobipocket::Document::InputSource, String, QLatin1String("Input Source")},
    {Mobipocket::Document::CreatorBuildRevision, String, QLatin1String("Kindlegen BuildRev Number")},
    {Mobipocket::Document::TamperKeys, Binary, QLatin1String("Tamper Proof Keys")},
    {Mobipocket::Document::FontSignature, Binary, QLatin1String("Font Signature")},
    {Mobipocket::Document::ReadForFree, Binary, QLatin1String("Read For Free")},
    {Mobipocket::Document::Unknown403, Binary, QLatin1String("Unknown (403)")},
    {Mobipocket::Document::Unknown407, Binary, QLatin1String("Unknown (407)")},
    {Mobipocket::Document::Unknown450, Binary, QLatin1String("Unknown (450)")},
    {Mobipocket::Document::Unknown451, Binary, QLatin1String("Unknown (451)")},
    {Mobipocket::Document::Unknown452, Binary, QLatin1String("Unknown (452)")},
    {Mobipocket::Document::Unknown453, Binary, QLatin1String("Unknown (453)")},
});

struct MobiHeader {
    /* MOBI header, offset 16 */
    QByteArray mobiMagic; /**< 16: M O B I { 77, 79, 66, 73 } */
    std::optional<quint32> headerLength; /**< 20: the length of the MOBI header, including the previous 4 bytes */
    std::optional<quint32> mobiType; /**< 24: mobipocket file type */
    std::optional<quint32> textEncoding; /**< 28: 1252 = CP1252, 65001 = UTF-8 */
    std::optional<quint32> uid; /**< 32: unique id */
    std::optional<quint32> version; /**< 36: mobipocket format */
    std::optional<quint32> orthIndex; /**< 40: section number of orthographic meta index. */
    std::optional<quint32> inflIndex; /**< 44: section number of inflection meta index. */
    std::optional<quint32> namesIndex; /**< 48: section number of names meta index. */
    std::optional<quint32> keysIndex; /**< 52: section number of keys meta index. */
    std::optional<quint32> extra0Index; /**< 56: section number of extra 0 meta index. */
    std::optional<quint32> extra1Index; /**< 60: section number of extra 1 meta index. */
    std::optional<quint32> extra2Index; /**< 64: section number of extra 2 meta index. */
    std::optional<quint32> extra3Index; /**< 68: section number of extra 3 meta index. */
    std::optional<quint32> extra4Index; /**< 72: section number of extra 4 meta index. */
    std::optional<quint32> extra5Index; /**< 76: section number of extra 5 meta index. */
    std::optional<quint32> nonTextIndex; /**< 80: first record number (starting with 0) that's not the book's text */
    std::optional<quint32> fullNameOffset; /**< 84: offset in record 0 (not from start of file) of the full name of the book */
    std::optional<quint32> fullNameLength; /**< 88: length of the full name */
    std::optional<quint32> locale; /**< 92: first byte is main language: 09 = English, next byte is dialect, 08 = British, 04 = US */
    std::optional<quint32> dictInputLang; /**< 96: input language for a dictionary */
    std::optional<quint32> dictOutputLang; /**< 100: output language for a dictionary */
    std::optional<quint32> minVersion; /**< 104: minimum mobipocket version support needed to read this file. */
    std::optional<quint32> imageIndex; /**< 108: first record number (starting with 0) that contains an image (sequential) */
    std::optional<quint32> huffRecIndex; /**< 112: first huffman compression record */
    std::optional<quint32> huffRecCount; /**< 116: huffman compression records count */
    std::optional<quint32> datpRecIndex; /**< 120: section number of DATP record */
    std::optional<quint32> datpRecCount; /**< 124: DATP records count */
    std::optional<quint32> exthFlags; /**< 128: bitfield. if bit 6 (0x40) is set, then there's an EXTH record */
    /* 32 unknown bytes, usually 0, related to encryption and unknown6 */
    /* unknown2 */
    /* unknown3 */
    /* unknown4 */
    /* unknown5 */
    std::optional<quint32> unknown6; /**< 164: use MOBI_NOTSET , related to encryption*/
    std::optional<quint32> drmOffset; /**< 168: offset to DRM key info in DRMed files. MOBI_NOTSET if no DRM */
    std::optional<quint32> drmCount; /**< 172: number of entries in DRM info */
    std::optional<quint32> drmSize; /**< 176: number of bytes in DRM info */
    std::optional<quint32> drmFlags; /**< 180: some flags concerning DRM info, bit 0 set if password encryption */
    /* 8 unknown bytes 0? */
    /* unknown7 */
    /* unknown8 */
    std::optional<quint16> firstTextIndex; /**< 192: section number of first text record */
    std::optional<quint16> lastTextIndex; /**< 194: */
    std::optional<quint32> fdstIndex; /**< 192 (KF8) section number of FDST record */
    // std::optional<quint32> unknown9; /**< 196: */
    std::optional<quint32> fdstSectionCount; /**< 196 (KF8) */
    std::optional<quint32> fcisIndex; /**< 200: section number of FCIS record */
    std::optional<quint32> fcisCount; /**< 204: FCIS records count */
    std::optional<quint32> flisIndex; /**< 208: section number of FLIS record */
    std::optional<quint32> flisCount; /**< 212: FLIS records count */
    std::optional<quint32> unknown10; /**< 216: */
    std::optional<quint32> unknown11; /**< 220: */
    std::optional<quint32> srcsIndex; /**< 224: section number of SRCS record */
    std::optional<quint32> srcsCount; /**< 228: SRCS records count */
    std::optional<quint32> unknown12; /**< 232: */
    std::optional<quint32> unknown13; /**< 236: */
    /* quint16 fill 0 */
    std::optional<quint16> extraFlags; /**< 242: extra flags */
    std::optional<quint32> ncxIndex; /**< 244: section number of NCX record  */
    std::optional<quint32> unknown14; /**< 248: */
    std::optional<quint32> fragmentIndex; /**< 248 (KF8) section number of fragments record */
    std::optional<quint32> unknown15; /**< 252: */
    std::optional<quint32> skeletonIndex; /**< 252 (KF8) section number of SKEL record */
    std::optional<quint32> datpIndex; /**< 256: section number of DATP record */
    std::optional<quint32> unknown16; /**< 260: */
    std::optional<quint32> guideIndex; /**< 260 (KF8) section number of guide record */
    std::optional<quint32> unknown17; /**< 264: */
    std::optional<quint32> unknown18; /**< 268: */
    std::optional<quint32> unknown19; /**< 272: */
    std::optional<quint32> unknown20; /**< 276: */
    QByteArray fullName; /**< variable offset (fullNameOffset): full name */
};
}

namespace Mobipocket
{

struct DocumentPrivate 
{
    DocumentPrivate(QIODevice *d)
        : pdbFile()
        , valid(true)
        , firstImageRecord(0)
        , drm(false)
    {
        pdbFile.read(d);
    }
    KPDBFile pdbFile;
    MobiHeader mobiHeader;
    std::unique_ptr<Decompressor> dec;
    quint16 ntextrecords;
    quint16 maxRecordSize;
    bool valid;
    bool isKF8;

    // number of first record holding image. Usually it is directly after end of text, but not always
    quint16 firstImageRecord;
    QMap<Document::MetaKey, QVariant> metadata;
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder toUtf16;
#else
    QTextCodec *codec = nullptr;
#endif
    bool drm;

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

static void read16(const QByteArray &data, qsizetype offset, std::optional<quint16> &value)
{
    if (data.size() <= offset + 2) {
        value = std::nullopt;
        return;
    }

    value = (quint16)((quint8)data[offset] << 8 | (quint8)data[offset + 1]);
}

static void read32(const QByteArray &data, qsizetype offset, std::optional<quint32> &value)
{
    if (data.size() <= offset + 4) {
        value = std::nullopt;
        return;
    }

    value = (quint32)((quint8)data[offset] << 24 | (quint8)data[offset + 1] << 16 | (quint8)data[offset + 2] << 8 | (quint8)data[offset + 3]);
}

void DocumentPrivate::init()
{
    valid = pdbFile.isValid();

    if (!valid)
        return;
    const QByteArray mhead = pdbFile.recordAt(0);
    if (mhead.isNull() || mhead.size() < 14)
        goto fail;
    dec = Decompressor::create(mhead[1], pdbFile);
    if ((int)mhead[12] != 0 || (int)mhead[13] != 0)
        drm = true;
    if (!dec) {
        goto fail;
    }

    mobiHeader.mobiMagic = mhead.mid(16, 4);
    Q_ASSERT(mobiHeader.mobiMagic == "MOBI");
    read32(mhead, 20, mobiHeader.headerLength);
    read32(mhead, 24, mobiHeader.mobiType);
    read32(mhead, 28, mobiHeader.textEncoding);
    Q_ASSERT(*mobiHeader.textEncoding == 1252 || *mobiHeader.textEncoding == 65001);

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    if (!mobiHeader.textEncoding || *mobiHeader.textEncoding == 65001) {
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
    if (!mobiHeader.textEncoding || *mobiHeader.textEncoding == 65001)
        codec = QTextCodec::codecForName("UTF-8");
    else
        codec = QTextCodec::codecForName("CP1252");
#endif

    read32(mhead, 32, mobiHeader.uid);
    read32(mhead, 36, mobiHeader.version);

    if (*mobiHeader.headerLength >= MOBI_HEADER_V7_SIZE && mobiHeader.version && *mobiHeader.version == 8) {
        isKF8 = 1;
    }

    read32(mhead, 40, mobiHeader.orthIndex);
    read32(mhead, 44, mobiHeader.inflIndex);
    read32(mhead, 48, mobiHeader.namesIndex);
    read32(mhead, 52, mobiHeader.keysIndex);
    read32(mhead, 56, mobiHeader.extra0Index);
    read32(mhead, 60, mobiHeader.extra1Index);
    read32(mhead, 64, mobiHeader.extra2Index);
    read32(mhead, 68, mobiHeader.extra3Index);
    read32(mhead, 72, mobiHeader.extra4Index);
    read32(mhead, 76, mobiHeader.extra5Index);
    read32(mhead, 80, mobiHeader.nonTextIndex);
    read32(mhead, 84, mobiHeader.fullNameOffset);
    read32(mhead, 88, mobiHeader.fullNameLength);
    read32(mhead, 92, mobiHeader.locale);
    read32(mhead, 96, mobiHeader.dictInputLang);
    read32(mhead, 100, mobiHeader.dictOutputLang);
    read32(mhead, 104, mobiHeader.minVersion);
    read32(mhead, 108, mobiHeader.imageIndex);
    read32(mhead, 112, mobiHeader.huffRecIndex);
    read32(mhead, 116, mobiHeader.huffRecCount);
    read32(mhead, 120, mobiHeader.datpRecIndex);
    read32(mhead, 124, mobiHeader.datpRecCount);
    read32(mhead, 128, mobiHeader.exthFlags);

    // 32 unknown bytes

    read32(mhead, 164, mobiHeader.unknown6);
    read32(mhead, 168, mobiHeader.drmOffset);
    read32(mhead, 172, mobiHeader.drmCount);
    read32(mhead, 176, mobiHeader.drmSize);

    // 8 unknown bytes

    if (isKF8) {
        read32(mhead, 192, mobiHeader.fdstIndex);
    } else {
        read16(mhead, 192, mobiHeader.firstTextIndex);
        read16(mhead, 194, mobiHeader.lastTextIndex);
    }
    read32(mhead, 196, mobiHeader.fdstSectionCount);
    read32(mhead, 200, mobiHeader.fcisIndex);
    read32(mhead, 204, mobiHeader.fcisCount);
    read32(mhead, 208, mobiHeader.flisIndex);
    read32(mhead, 212, mobiHeader.flisCount);
    read32(mhead, 216, mobiHeader.unknown10);
    read32(mhead, 220, mobiHeader.unknown11);
    read32(mhead, 224, mobiHeader.srcsIndex);
    read32(mhead, 228, mobiHeader.srcsCount);
    read32(mhead, 232, mobiHeader.unknown12);
    read32(mhead, 236, mobiHeader.unknown13);

    // skip 2 bytes

    read16(mhead, 242, mobiHeader.extraFlags);
    read32(mhead, 244, mobiHeader.ncxIndex);
    if (isKF8) {
        read32(mhead, 248, mobiHeader.fragmentIndex);
        read32(mhead, 252, mobiHeader.skeletonIndex);
    } else {
        read32(mhead, 248, mobiHeader.unknown14);
        read32(mhead, 252, mobiHeader.unknown15);
    }
    read32(mhead, 256, mobiHeader.datpIndex);
    if (isKF8) {
        read32(mhead, 260, mobiHeader.guideIndex);
    } else {
        read32(mhead, 260, mobiHeader.unknown16);
    }
    read32(mhead, 264, mobiHeader.unknown17);
    read32(mhead, 268, mobiHeader.unknown18);
    read32(mhead, 272, mobiHeader.unknown19);
    read32(mhead, 276, mobiHeader.unknown20);

    if (mobiHeader.fullNameOffset && mobiHeader.fullNameLength) {
        const quint32 fullNameLength = std::min(*mobiHeader.fullNameLength, MOBI_TITLE_SIZEMAX);
        if (fullNameLength) {
            mobiHeader.fullName = mhead.mid(280, fullNameLength);
        }
    }

    ntextrecords = (unsigned char)mhead[8];
    ntextrecords <<= 8;
    ntextrecords += (unsigned char)mhead[9];
    maxRecordSize = (unsigned char)mhead[10];
    maxRecordSize <<= 8;
    maxRecordSize += (unsigned char)mhead[11];
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
    if (mobiHeader.imageIndex) {
        firstImageRecord = *mobiHeader.imageIndex;
    } else {
        firstImageRecord = ntextrecords + 1;
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
            metadata[Document::Title] = QString(toUtf16(data.mid(nameoffset, namelen)));
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
        auto type = static_cast<Document::MetaKey>(readBELong(data, offset));
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
            quint32 len = readBELong(data, offset);
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
                        QDateTime::fromString(date.first(date.indexOf(QStringLiteral(".")) + 4) + date.mid(date.indexOf(QStringLiteral(".")) + 7),
                                              QStringLiteral("yyyy-MM-dd HH:mm:ss.zzzttt"));
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

}
