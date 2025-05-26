// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-Licence-Identifier: BSD-2-Clauses

#include "../lib/mobipocket.h"
#include <QFile>
#include <QObject>
#include <QTest>

class MetadataTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMetadataKF8()
    {
        QFile file(QStringLiteral(DATA_DIR "/first-men-in-the-moon.azw3"));
        QVERIFY(file.open(QIODevice::ReadOnly));

        Mobipocket::Document document(&file);
        const auto metadata = document.metadata();
        QCOMPARE(metadata[Mobipocket::Document::Title].toString(), QStringLiteral("The First Men in the Moon"));
        QCOMPARE(metadata[Mobipocket::Document::Author].toString(), QStringLiteral("H. G. Wells"));
        QCOMPARE(metadata[Mobipocket::Document::Publisher].toString(), QStringLiteral("Standard Ebooks"));
        QCOMPARE(metadata[Mobipocket::Document::Description].toString(), QStringLiteral("Two British men become the first humans to reach the Moon."));
        QCOMPARE(
            metadata[Mobipocket::Document::Subject].toString(),
            QStringLiteral(
                "Science fiction; Satire; Utopias -- Fiction; Imperialism -- Fiction; Utopian fiction; Space flight to the moon -- Fiction; Moon -- Fiction"));
        QCOMPARE(metadata[Mobipocket::Document::PublishingDate].toDateTime(), QDateTime(QDate(2025, 04, 28), QTime(18, 16, 24, 255), QTimeZone::UTC));
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
        const auto metadata = document.metadata();

        QCOMPARE(metadata[Mobipocket::Document::Title].toString(), QStringLiteral("The First Men in the Moon"));
        QCOMPARE(metadata[Mobipocket::Document::Author].toString(), QStringLiteral("H. G. Wells"));
        QCOMPARE(metadata[Mobipocket::Document::Publisher].toString(), QStringLiteral("Standard Ebooks"));
        QCOMPARE(metadata[Mobipocket::Document::Description].toString(), QStringLiteral("Two British men become the first humans to reach the Moon."));
        QCOMPARE(
            metadata[Mobipocket::Document::Subject].toString(),
            QStringLiteral(
                "Science fiction; Satire; Utopias -- Fiction; Imperialism -- Fiction; Utopian fiction; Space flight to the moon -- Fiction; Moon -- Fiction"));
        QCOMPARE(metadata[Mobipocket::Document::PublishingDate].toDateTime(), QDateTime(QDate(2025, 04, 28), QTime(18, 16, 24, 255), QTimeZone::UTC));
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
};

QTEST_MAIN(MetadataTest)
#include "metadatatest.moc"
