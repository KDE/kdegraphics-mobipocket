/*
    SPDX-FileCopyrightText: 2025 Stefan Brüns <stefan.bruens@rwth-aachen.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "../lib/bitreader_p.h"

#include <QTest>

using namespace Mobipocket;

class BitReaderTest : public QObject
{
    Q_OBJECT
public:
    void setLocalized(bool);

private Q_SLOTS:
    void testRead_1();
    void testRead_2();
    void testRead1Bit();
    void testRead8Bit();
    void benchmarkInit();
    void benchmarkInitSlice();
    void benchmarkRead1Bit();
    void benchmarkRead8Bit();
};

void BitReaderTest::testRead_1()
{
    QByteArray data(1, '\x01');

    BitReader r(data);

    QCOMPARE(r.left(), 8);
    QCOMPARE(r.read(), 0x01000000);
    QVERIFY(r.eat(4));

    QCOMPARE(r.left(), 4);
    QCOMPARE(r.read(), 0x10000000);
    QVERIFY(r.eat(2));

    QCOMPARE(r.left(), 2);
    QCOMPARE(r.read(), 0x40000000);
    QVERIFY(r.eat(1));

    QCOMPARE(r.left(), 1);
    QCOMPARE(r.read(), 0x80000000);
    QVERIFY(r.eat(1));

    QCOMPARE(r.left(), 0);
    QCOMPARE(r.read(), 0x00000000);
    QVERIFY(!r.eat(1));
}

void BitReaderTest::testRead_2()
{
    QByteArray data("\x01\xff\xaa\x81", 4);

    BitReader r(data);

    QCOMPARE(r.left(), 32);
    QCOMPARE(r.read(), 0x01ffaa81);
    QVERIFY(r.eat(4));

    QCOMPARE(r.left(), 28);
    QCOMPARE(r.read(), 0x1ffaa810);
    QVERIFY(r.eat(2));

    QCOMPARE(r.left(), 26);
    QCOMPARE(r.read(), 0x7feaa040);
    QVERIFY(r.eat(1));

    QCOMPARE(r.left(), 25);
    QCOMPARE(r.read(), 0xffd54080);
    QVERIFY(r.eat(1));

    QCOMPARE(r.left(), 24);
    QCOMPARE(r.read(), 0xffaa8100);
    QVERIFY(r.eat(16));

    QCOMPARE(r.left(), 8);
    QCOMPARE(r.read(), 0x81000000);
    QVERIFY(r.eat(4));

    QCOMPARE(r.left(), 4);
    QCOMPARE(r.read(), 0x10000000);
    QVERIFY(r.eat(3));

    QCOMPARE(r.left(), 1);
    QCOMPARE(r.read(), 0x80000000);
    QVERIFY(r.eat(1));

    QCOMPARE(r.left(), 0);
    QCOMPARE(r.read(), 0x00000000);
    QVERIFY(!r.eat(1));
}

void BitReaderTest::testRead1Bit()
{
    QByteArray data(128, '\x01');

    size_t count = 0;
    BitReader r(data);
    while (r.left()) {
        count++;
        r.read();
        r.eat(1);
    }
    QCOMPARE(count, 1024);

    QCOMPARE(r.left(), 0);
    QCOMPARE(r.read(), 0x00000000);
    QVERIFY(!r.eat(1));
}

void BitReaderTest::testRead8Bit()
{
    QByteArray data(1024, '\x01');

    size_t count = 0;
    BitReader r(data);
    while (r.left() > 24) {
        count++;
        auto t = r.read();
        QCOMPARE(t, 0x01010101);
        r.eat(8);
    }
    QCOMPARE(count, 1021);

    QCOMPARE(r.read(), 0x01010100);
    QVERIFY(r.eat(8));
    QCOMPARE(r.read(), 0x01010000);
    QVERIFY(r.eat(8));
    QCOMPARE(r.read(), 0x01000000);
    QVERIFY(r.eat(8));
    QCOMPARE(r.left(), 0);
    QCOMPARE(r.read(), 0x00000000);
    QVERIFY(!r.eat(1));
}

void BitReaderTest::benchmarkInit()
{
    QByteArray data(1024, '\0');

    QBENCHMARK {
        BitReader r(data);
    }
}

void BitReaderTest::benchmarkInitSlice()
{
    QByteArray data(1024, '\0');

    QBENCHMARK {
        BitReader r(data.mid(1));
    }
}

void BitReaderTest::benchmarkRead1Bit()
{
    QByteArray data(128, '\0');

    QBENCHMARK {
        BitReader r(data);
        while (r.left()) {
            r.read();
            r.eat(1);
        }
    }
}

void BitReaderTest::benchmarkRead8Bit()
{
    QByteArray data(1024, '\0');

    QBENCHMARK {
        BitReader r(data);
        while (r.left()) {
            r.read();
            r.eat(8);
        }
    }
}

QTEST_GUILESS_MAIN(BitReaderTest)

#include "bitreadertest.moc"
