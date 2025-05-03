// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MOBIPOCKET_H
#define MOBIPOCKET_H

#include <QByteArray>
#include <QImage>
#include <QMap>
#include <QString>

#include "qmobipocket_export.h"

class QIODevice;
class MetadataTest;

namespace Mobipocket
{

struct MobiHeader;
struct DocumentPrivate;
class QMOBIPOCKET_EXPORT Document
{
    Q_GADGET
public:
    enum MetaKey {
        DrmServer = 1,
        DrmCommerce = 2,
        DrmBookbase = 3,
        Title = 99, /**< <dc:title> */
        Author = 100, /**< <dc:creator> */
        Publisher = 101, /**< <dc:publisher> */
        Imprint = 102, /**< <imprint> */
        Description = 103, /**< <dc:description> */
        ISBN = 104, /**< <dc:identifier opf:scheme="ISBN"> */
        Subject = 105, /**< <dc:subject> */
        PublishingDate = 106, /**< <dc:date> */
        Review = 107, /**< <review> */
        Contributor = 108, /**< <dc:contributor> */
        Rights = 109, /**< <dc:rights> */
        Copyright = Rights, /// \deprecated
        SubjectCode = 110, /**< <dc:subject BASICCode="subjectcode"> */
        Type = 111, /**< <dc:type> */
        Source = 112, /**< <dc:source> */
        ASIN = 113,
        Version = 114,
        Sample = 115,
        StartThreading = 116, /**< Start reading */
        Adult = 117, /**< <adult> */
        Price = 118, /**< <srp> */
        Currency = 119, /**< <srp currency="currency"> */
        KF8Boundary = 121,
        FixedLayout = 122, /**< <fixed-layout> */
        BookType = 123, /**< <book-type> */
        OrientationLock = 124, /**< <orientation-lock> */
        CountResources = 125,
        OriginalResolution = 126, /**< <original-resolution> */
        ZeroGutter = 127, /**< <zero-gutter> */
        ZeroMargin = 128, /**< <zero-margin> */
        KF8CoverUri = 129,
        RESCOffset = 131,
        RegionMag = 132, /**< <region-mag> */

        DictionaryName = 200, /**< <DictionaryVeryShortName> */
        CoverOffset = 201, /**< <EmbeddedCover> */
        ThumbnailOffset = 202,
        HasFakeCover = 203,
        CreatorSoftware = 204,
        CreatorMajorVersion = 205,
        CreatorMinorVersion = 206,
        CreatorBuild = 207,
        Watermark = 208,
        TamperKeys = 209,

        FontSignature = 300,

        ClippingLimit = 401,
        PublisherLimit = 402,
        Unknown403 = 403,
        TTSDisable = 404,
        ReadForFree = 405, // uint32_t, rental related, ReadForFree
        Rental = 406, // uint64_t
        Unknown407 = 407,
        Unknown450 = 450,
        Unknown451 = 451,
        Unknown452 = 452,
        Unknown453 = 453,

        Doctype = 501, /**< PDOC - Personal Doc; EBOK - ebook; EBSP - ebook sample; */
        LastUpdate = 502,
        UpdatedTitle = 503,
        ASIN504 = 504,
        TitleFileAs = 508,
        CreatorFileAs = 517,
        PublisherFileAs = 522,
        Language = 524, /**< <dc:language> */
        Alignment = 525, /**< <primary-writing-mode> */
        CreatorString = 526,
        PageDir = 527,
        OverrideKindleFonts = 528, /**< <override-kindle-fonts> */
        OriginalSourceDescription = 529,
        DictionaryInputLanguage = 531,
        DictionaryOutputLanguage = 532,
        InputSource = 534,
        CreatorBuildRevision = 535,
    };
    Q_ENUM(MetaKey);

    /**
     * Mobipocket::Document constructor
     *
     * @params device The IO device corresponding to the mobipocket document. The device must
     * be open for read operations, not sequential and the document does not take ownership of
     * the device.
     */
    explicit Document(QIODevice *device);
    virtual ~Document();

    QMap<MetaKey, QVariant> metadata() const;
    QString text(int size=-1) const;
    int imageCount() const;
    QImage getImage(int i) const;
    QImage thumbnail() const;
    bool isValid() const;

    // if true then it is impossible to get text of book. Images should still be readable
    bool hasDRM() const;

    Q_DISABLE_COPY(Document);
private:
    friend MetadataTest;
    const MobiHeader &mobiHeader() const;

    DocumentPrivate *const d;
};
}
#endif
