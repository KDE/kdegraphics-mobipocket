/*
    SPDX-FileCopyrightText: 2025 Stefan Brüns <stefan.bruens@rwth-aachen.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "../lib/decompressor.h"

#include <QBuffer>
#include <QTest>

using namespace Mobipocket;

class DecompressorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testNoop();
    void testNoop_data();
    void testRLE();
    void testRLE_data();
    void testHuffInit();
};

void DecompressorTest::testNoop()
{
    QFETCH(QByteArray, data);

    auto decompressor = Decompressor::create(1, {});

    auto r = decompressor->decompress(data);
    // NOOP -> input and output are identical
    QCOMPARE(r, data);
}

void DecompressorTest::testNoop_data()
{
    QTest::addColumn<QByteArray>("data");

    QTest::addRow("empty") << QByteArray();
    QTest::addRow("0x00 * 10") << QByteArray(10, '\x00');
    QTest::addRow("0xaa * 10") << QByteArray(10, '\xaa');
}

void DecompressorTest::testRLE()
{
    QFETCH(QByteArray, data);
    QFETCH(QByteArray, expected);

    auto decompressor = Decompressor::create(2, {});

    QEXPECT_FAIL("repeat", "broken end check", Abort);
    QEXPECT_FAIL("repeat 2", "broken end check", Abort);
    auto r = decompressor->decompress(data);
    QCOMPARE(r, expected);
}

void DecompressorTest::testRLE_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("expected");

    QTest::addRow("empty") << QByteArray() << QByteArray();
    // Token '0x00' is passed verbatim
    QTest::addRow("0x00 * 10") << QByteArray(10, '\x00') << QByteArray(10, '\x00');

    // Tokens in the range 0x09..0x7f are passed verbatim
    QTest::addRow("0x20 * 20") << QByteArray(20, '\x20') << QByteArray(20, '\x20');
    QTest::addRow("0x7f * 20") << QByteArray(20, '\x7f') << QByteArray(20, '\x7f');

    // Tokens in the range 0xc0..0xff are expanded to " \x40".." \x7f"
    QTest::addRow("0xc0 * 64") << QByteArray(64, '\xc0') << QByteArray(" \x40", 2).repeated(64);
    QTest::addRow("0xf0 * 64") << QByteArray(64, '\xf0') << QByteArray(" \x70", 2).repeated(64);

    QTest::addRow("repeat") << QByteArray("\x32\x80\x0a", 3) << QByteArray(6, '2');
    QTest::addRow("repeat 2") << QByteArray("\x31\x65\x80\x13", 4) << QByteArray("1e").repeated(4);
}

void DecompressorTest::testHuffInit()
{
    {
        auto decompressor = Decompressor::create('H', {});
        QVERIFY(!decompressor->isValid());
    }
    {
        auto decompressor = Decompressor::create('H', QVector<QByteArray>(2));
        QVERIFY(!decompressor->isValid());
    }
    {
        QByteArray HDic(512, '\0');
        QByteArray CDic(512, '\0');

        auto decompressor = Decompressor::create('H', {HDic, CDic});
        QVERIFY(!decompressor->isValid());
    }
    {
        QByteArray HDic("HUFF", 4);
        QByteArray CDic("CDIC", 4);
        QByteArray fill(60, '\0');

        auto decompressor = Decompressor::create('H', {HDic + fill, CDic + fill});
        QVERIFY(!decompressor->isValid());
    }
}

QTEST_GUILESS_MAIN(DecompressorTest)

#include "decompressortest.moc"
