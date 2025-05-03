// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-Licence-Identifier: LGPL-2.1-or-later

#include "../lib/mobiheader_p.h"
#include "../lib/mobipocket.h"
#include <QFile>
#include <QObject>
#include <QTest>
#include <QTimeZone>

class MetadataTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMetadataKF8()
    {
        QFile file(QStringLiteral(DATA_DIR "/first-men-in-the-moon.azw3"));
        QVERIFY(file.open(QIODevice::ReadOnly));

        Mobipocket::Document document(&file);
        QVERIFY(document.isValid());

        const auto metadata = document.metadata();
        QCOMPARE(metadata[Mobipocket::Document::Title].toString(), QStringLiteral("The First Men in the Moon"));
        QCOMPARE(metadata[Mobipocket::Document::Author].toString(), QStringLiteral("H. G. Wells"));
        QCOMPARE(metadata[Mobipocket::Document::Publisher].toString(), QStringLiteral("Standard Ebooks"));
        QCOMPARE(metadata[Mobipocket::Document::Description].toString(), QStringLiteral("Two British men become the first humans to reach the Moon."));
        QCOMPARE(
            metadata[Mobipocket::Document::Subject].toString(),
            QStringLiteral(
                "Science fiction; Satire; Utopias -- Fiction; Imperialism -- Fiction; Utopian fiction; Space flight to the moon -- Fiction; Moon -- Fiction"));
        QCOMPARE(metadata[Mobipocket::Document::PublishingDate].toDateTime(), QDateTime(QDate(2025, 04, 28), QTime(18, 16, 24, 255), QTimeZone::utc()));
        QCOMPARE(metadata[Mobipocket::Document::Contributor].toString(), QStringLiteral("calibre (8.0.1) [https://calibre-ebook.com]"));
        QCOMPARE(metadata[Mobipocket::Document::Source].toString(), QStringLiteral("calibre:7615562a-570a-432e-a70c-2e00b3294077"));
        QCOMPARE(metadata[Mobipocket::Document::ASIN].toString(), QStringLiteral("7615562a-570a-432e-a70c-2e00b3294077"));
        QCOMPARE(metadata[Mobipocket::Document::KF8CoverUri].toString(), QStringLiteral("kindle:embed:0001"));
        QCOMPARE(metadata[Mobipocket::Document::Doctype].toString(), QStringLiteral("EBOK"));
        QCOMPARE(metadata[Mobipocket::Document::UpdatedTitle].toString(), QStringLiteral("The First Men in the Moon"));
        QCOMPARE(metadata[Mobipocket::Document::Language].toString(), QStringLiteral("en"));
        QCOMPARE(metadata[Mobipocket::Document::OverrideKindleFonts].toString(), QStringLiteral("true"));
        QCOMPARE(metadata[Mobipocket::Document::CreatorBuildRevision].toString(), QStringLiteral("0730-890adc2"));
        QCOMPARE(metadata[Mobipocket::Document::CreatorBuildRevision].toInt(), 0);
        QCOMPARE(metadata[Mobipocket::Document::CreatorMajorVersion].toInt(), 2);
        QCOMPARE(metadata[Mobipocket::Document::CreatorMinorVersion].toInt(), 9);
        QCOMPARE(metadata[Mobipocket::Document::HasFakeCover].toInt(), 0);
        QCOMPARE(metadata[Mobipocket::Document::RESCOffset].toInt(), 0);
        QCOMPARE(metadata[Mobipocket::Document::CoverOffset].toInt(), 0);
        QCOMPARE(metadata[Mobipocket::Document::ThumbnailOffset].toInt(), 1);
        QCOMPARE(metadata[Mobipocket::Document::CountResources].toInt(), 2);

        QCOMPARE(document.thumbnail().width(), 169);
        QCOMPARE(document.thumbnail().height(), 240);
    }

    void testMetadataMobi()
    {
        QFile file(QStringLiteral(DATA_DIR "/first-men-in-the-moon.mobi"));
        QVERIFY(file.open(QIODevice::ReadOnly));

        Mobipocket::Document document(&file);
        QVERIFY(document.isValid());

        const auto metadata = document.metadata();

        QCOMPARE(metadata[Mobipocket::Document::Title].toString(), QStringLiteral("The First Men in the Moon"));
        QCOMPARE(metadata[Mobipocket::Document::Author].toString(), QStringLiteral("H. G. Wells"));
        QCOMPARE(metadata[Mobipocket::Document::Publisher].toString(), QStringLiteral("Standard Ebooks"));
        QCOMPARE(metadata[Mobipocket::Document::Description].toString(), QStringLiteral("Two British men become the first humans to reach the Moon."));
        QCOMPARE(
            metadata[Mobipocket::Document::Subject].toString(),
            QStringLiteral(
                "Science fiction; Satire; Utopias -- Fiction; Imperialism -- Fiction; Utopian fiction; Space flight to the moon -- Fiction; Moon -- Fiction"));
        QCOMPARE(metadata[Mobipocket::Document::PublishingDate].toDateTime(), QDateTime(QDate(2025, 04, 28), QTime(18, 16, 24, 255), QTimeZone::utc()));
        QCOMPARE(metadata[Mobipocket::Document::Contributor].toString(), QStringLiteral("calibre (8.0.1) [https://calibre-ebook.com]"));
        QCOMPARE(metadata[Mobipocket::Document::Source].toString(), QStringLiteral("calibre:7615562a-570a-432e-a70c-2e00b3294077"));
        QCOMPARE(metadata[Mobipocket::Document::ASIN].toString(), QStringLiteral("7615562a-570a-432e-a70c-2e00b3294077"));
        QCOMPARE(metadata[Mobipocket::Document::KF8CoverUri].toString(), QStringLiteral("kindle:embed:0001"));
        QCOMPARE(metadata[Mobipocket::Document::Doctype].toString(), QStringLiteral("EBOK"));
        QCOMPARE(metadata[Mobipocket::Document::UpdatedTitle].toString(), QStringLiteral("The First Men in the Moon"));
        QCOMPARE(metadata[Mobipocket::Document::Language].toString(), QStringLiteral("en"));
        QCOMPARE(metadata[Mobipocket::Document::OverrideKindleFonts].toString(), QStringLiteral("true"));
        QCOMPARE(metadata[Mobipocket::Document::CreatorBuildRevision].toInt(), 0);
        QCOMPARE(metadata[Mobipocket::Document::CreatorMajorVersion].toInt(), 1);
        QCOMPARE(metadata[Mobipocket::Document::CreatorMinorVersion].toInt(), 2);
        QCOMPARE(metadata[Mobipocket::Document::HasFakeCover].toInt(), 0);
        QCOMPARE(metadata[Mobipocket::Document::StartThreading].toInt(), 109);
        QCOMPARE(metadata[Mobipocket::Document::RESCOffset].toInt(), 0);
        QCOMPARE(metadata[Mobipocket::Document::CoverOffset].toInt(), 0);
        QCOMPARE(metadata[Mobipocket::Document::ThumbnailOffset].toInt(), 1);
        QCOMPARE(metadata[Mobipocket::Document::CountResources].toInt(), 0);

        QCOMPARE(document.thumbnail().width(), 169);
        QCOMPARE(document.thumbnail().height(), 240);
    }

    void testMobiHeader()
    {
        QFile file(QStringLiteral(DATA_DIR "/first-men-in-the-moon.mobi"));
        QVERIFY(file.open(QIODevice::ReadOnly));

        Mobipocket::Document document(&file);
        const auto mobiHeader = document.mobiHeader();

        QCOMPARE(mobiHeader.mobiMagic, "MOBI");
        QCOMPARE(mobiHeader.headerLength, 232);
        QCOMPARE(mobiHeader.mobiType, Mobipocket::MobiHeader::MobiType::MobiBook);
        QCOMPARE(mobiHeader.textEncoding, 65001);
        QCOMPARE(mobiHeader.uid, 2371055849);
        QCOMPARE(mobiHeader.version, 6);
        QCOMPARE(mobiHeader.orthIndex, 4294967295);
        QCOMPARE(mobiHeader.inflIndex, 4294967295);
        QCOMPARE(mobiHeader.namesIndex, 4294967295);
        QCOMPARE(mobiHeader.keysIndex, 4294967295);
        QCOMPARE(mobiHeader.extra0Index, 4294967295);
        QCOMPARE(mobiHeader.extra1Index, 4294967295);
        QCOMPARE(mobiHeader.extra2Index, 4294967295);
        QCOMPARE(mobiHeader.extra3Index, 4294967295);
        QCOMPARE(mobiHeader.extra4Index, 4294967295);
        QCOMPARE(mobiHeader.extra5Index, 4294967295);
        QCOMPARE(mobiHeader.nonTextIndex, 113);
        QCOMPARE(mobiHeader.fullNameOffset, 904);
        QCOMPARE(mobiHeader.fullNameLength, 25);
        QCOMPARE(mobiHeader.locale, 9);
        QCOMPARE(mobiHeader.dictInputLang, 0);
        QCOMPARE(mobiHeader.dictOutputLang, 0);
        QCOMPARE(mobiHeader.minVersion, 6);
        QCOMPARE(mobiHeader.imageIndex, 116);
        QCOMPARE(mobiHeader.huffRecIndex, 0);
        QCOMPARE(mobiHeader.huffRecCount, 0);
        QCOMPARE(mobiHeader.datpRecIndex, 0);
        QCOMPARE(mobiHeader.datpRecCount, 0);
        QCOMPARE(mobiHeader.exthFlags, 80);
        QCOMPARE(mobiHeader.drmOffset, 4294967295);
        QCOMPARE(mobiHeader.drmCount, 0);
        QCOMPARE(mobiHeader.drmSize, 0);
        QCOMPARE(mobiHeader.drmFlags, 0);
        QCOMPARE(*mobiHeader.firstTextIndex, 1);
        QCOMPARE(*mobiHeader.lastTextIndex, 119);
        QCOMPARE(mobiHeader.fdstIndex, std::nullopt);
        QCOMPARE(mobiHeader.fdstSectionCount, 1);
        QCOMPARE(mobiHeader.fcisIndex, 121);
        QCOMPARE(mobiHeader.fcisCount, 1);
        QCOMPARE(mobiHeader.flisIndex, 120);
        QCOMPARE(mobiHeader.flisCount, 1);
        QCOMPARE(mobiHeader.srcsIndex, 4294967295);
        QCOMPARE(mobiHeader.srcsCount, 0);
        QCOMPARE(mobiHeader.extraFlags, 3);
        QCOMPARE(mobiHeader.ncxIndex, 113);
    }
};

QTEST_MAIN(MetadataTest)
#include "metadatatest.moc"
