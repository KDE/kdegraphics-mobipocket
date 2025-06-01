/*
    SPDX-FileCopyrightText: 2025 Stefan Brüns <stefan.bruens@rwth-aachen.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mobipocket.h"
#include "testsconfig.h"

#include <QTest>

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

    const auto text = doc.text();
    const auto expected = QStringLiteral("<html><head></head><body>" //
        "<p height=\"1em\" width=\"0pt\">This is a sample PDF file for KFileMetaData. </p>" //
        "<mbp:pagebreak/><a ></a> <a ></a> <a ></a></body></html>");
    QCOMPARE(text, expected);
}

QTEST_GUILESS_MAIN(MobipocketTest)

#include "mobipockettest.moc"
