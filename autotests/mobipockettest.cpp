/*
    SPDX-FileCopyrightText: 2025 Stefan Brüns <stefan.bruens@rwth-aachen.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mobipocket.h"
#include "testsconfig.h"

#include <QTest>

#include <QBuffer>

using namespace Mobipocket;

namespace {
QString testFilePath(const QString& fileName)
{
    return QLatin1String(TESTS_FILES_PATH) + QLatin1Char('/') + fileName;
}
}

class MobipocketTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMetadata();
    void testText();
    void testThumbnail();
    void testTruncation();
};

void MobipocketTest::testMetadata()
{
    QFile file(testFilePath(QStringLiteral("test.mobi")));
    file.open(QFile::ReadOnly);
    Mobipocket::Document doc(&file);

    QVERIFY(doc.isValid());

    const auto metadata = doc.metadata();

    QCOMPARE(metadata.value(Document::Author), QStringLiteral("Happy Man"));
    QCOMPARE(metadata.value(Document::Title), QStringLiteral("The Big Brown Bear"));
    QCOMPARE(metadata.value(Document::Subject), QStringLiteral("Baloo KFileMetaData"));
    QCOMPARE(metadata.value(Document::Description), QStringLiteral("Honey"));
    QCOMPARE(metadata.value(Document::Copyright), QStringLiteral("License"));
}

void MobipocketTest::testText()
{
    QFile file(testFilePath(QStringLiteral("test.mobi")));
    file.open(QFile::ReadOnly);
    Mobipocket::Document doc(&file);

    QVERIFY(doc.isValid());
    QVERIFY(!doc.hasDRM());

    const auto text = doc.text();
    const auto expected = QStringLiteral("<html><head></head><body>" //
        "<p height=\"1em\" width=\"0pt\">This is a sample PDF file for KFileMetaData. </p>" //
        "<mbp:pagebreak/><a ></a> <a ></a> <a ></a></body></html>");
    QCOMPARE(text, expected);
}

void MobipocketTest::testThumbnail()
{
    QFile file(testFilePath(QStringLiteral("test.mobi")));
    file.open(QFile::ReadOnly);
    Mobipocket::Document doc(&file);

    QVERIFY(doc.isValid());

    const auto thumb = doc.thumbnail();
    QCOMPARE(thumb.width(), 179);
    QCOMPARE(thumb.height(), 233);

    QVERIFY(doc.imageCount() >= 2);
    // Thumbnail is second image
    QCOMPARE(thumb, doc.getImage(1));

    const auto cover = doc.getImage(0);
    QCOMPARE(cover.width(), 566);
    QCOMPARE(cover.height(), 734);

    // Should not crash
    const auto invalid1 = doc.getImage(doc.imageCount() + 1);
    QCOMPARE(invalid1.width(), 0);
    // Unfortunately allowed by API, fix occasionally and bump ABI version
    const auto invalid2 = doc.getImage(-10);
    QCOMPARE(invalid2.width(), 0);
}

void MobipocketTest::testTruncation()
{
    QFile file(testFilePath(QStringLiteral("test.mobi")));
    file.open(QFile::ReadOnly);
    auto data = file.readAll();

    QCOMPARE(data.size(), 13653);

    for (auto size = data.size(); size >= 0; size--) {
        QBuffer buf;
        buf.setData(data.constData(), size);
        buf.open(QIODevice::ReadOnly);

        Mobipocket::Document doc(&buf);
        const auto metadata = doc.metadata();
        const auto text = doc.text();
    }
}

QTEST_GUILESS_MAIN(MobipocketTest)

#include "mobipockettest.moc"
