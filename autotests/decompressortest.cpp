/*
    SPDX-FileCopyrightText: 2025 Stefan Brüns <stefan.bruens@rwth-aachen.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "../lib/decompressor.h"

#include <QBuffer>
#include <QTest>
#include <QVector>
#include <QtEndian>
#include <array>

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
    void testHuffDecompress();
    void testFuzzHuff();
    void benchmarkHuffDecompress();
};

namespace {
    QVector<QByteArray> createHuffIdentityDict()
    {
        // Create a Huffman dictionary which maps each input byte to itself
        static std::array<quint8, 256 * 4> hdict = []() {
            std::array<quint8, 256 * 4> d;
            for (size_t i = 0; i < d.size(); i += 4) {
                // 1. Codelen is 8 bits
                // 2. Only use the first tree dictionary, set the termination flag
                d[i] = 8 | 0x80;
                d[i + 1] = i / 2;
                d[i + 2] = i / 512;
            }
            return d;
        }();

        QByteArray huff("HUFF", 4);
        huff.resize(24);
        qToBigEndian<quint32>(huff.size(), huff.data() + 16);
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        huff.append(QByteArrayView(hdict));
#else
        huff.append(QByteArray::fromRawData(reinterpret_cast<char*>(hdict.data()), hdict.size()));
#endif
        qToBigEndian<quint32>(huff.size(), huff.data() + 20);
        huff.append(64 * 4, '\0');

        static std::array<quint8, 256 * (2 + 3)> entries = []() {
            std::array<quint8, 256 * (2 + 3)> d;
            for (size_t i = 0; i < 256; i++) {
                quint16 off = 512 + 3 * i;
                qToBigEndian<quint16>(off, &d[2 * i]);
                qToBigEndian<quint16>(0x8001, &d[off]); // len==1 | termination flag
                d[off + 2] = i;
            }
            return d;
        }();

        QByteArray cdic("CDIC\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16);
        qToBigEndian<quint32>(32, cdic.data() + 12);
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        cdic.append(QByteArrayView(entries));
#else
        cdic.append(QByteArray::fromRawData(reinterpret_cast<char*>(entries.data()), entries.size()));
#endif

        return {huff, cdic};
    }
}

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

    // Tokens in the range 0x01..0x08 denotes the length of raw copied data
    QTest::addRow("raw 0x01...") << QByteArray("\x01\xff", 2) << QByteArray("\xff", 1);
    QTest::addRow("raw .0x01...") << QByteArray("d\x01\xc0kj", 5) << QByteArray("d\xc0kj", 4);
    QTest::addRow("raw .0x02...") << QByteArray("d\x02\xc0kj", 5) << QByteArray("d\xc0kj", 4);
    QTest::addRow("raw .0x03...") << QByteArray("d\x03\xc0kj", 5) << QByteArray("d\xc0kj", 4);
    // Short data
    QTest::addRow("short 0x03...") << QByteArray("d\x03\xc0k", 4) << QByteArray("d", 1);

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
    {
        auto decompressor = Decompressor::create('H', createHuffIdentityDict());
        QVERIFY(decompressor->isValid());
    }
}

void DecompressorTest::testHuffDecompress()
{
    auto decompressor = Decompressor::create('H', createHuffIdentityDict());
    QVERIFY(decompressor->isValid());

    {
        auto r = decompressor->decompress(QByteArray("\0", 1));
        QCOMPARE(r, QByteArray("\0", 1));
    }
    {
        auto r = decompressor->decompress(QByteArray("\1\xcc", 2));
        QCOMPARE(r, QByteArray("\1\xcc", 2));
    }
    {
        QByteArray d(256, '\0');
        for (int i = 0; i < d.size(); i++) {
            d[i] = i;
        }
        auto r = decompressor->decompress(d);
        QCOMPARE(r, d);
    }
}

void DecompressorTest::benchmarkHuffDecompress()
{
    auto decompressor = Decompressor::create('H', createHuffIdentityDict());
    QVERIFY(decompressor->isValid());

    QByteArray data(1024, '\x01');

    QBENCHMARK {
        auto r = decompressor->decompress(data);
        QCOMPARE(r, data);
    }
}

void DecompressorTest::testFuzzHuff()
{
    auto verify = [](const auto &decompressor) {
        QByteArray d(256, '\0');
        for (int i = 0; i < d.size(); i++) {
            d[i] = i;
        }
        // The output does not matter and is likely
        // just garbage, but it should not crash
        auto r = decompressor->decompress(d);
    };

    auto dict = createHuffIdentityDict();

    for (auto i = dict.at(0).size() - 1; i >= 0; i--) {
        unsigned char originalValue = dict.at(0).at(i);

        {
            dict[0][i] = originalValue ^ 0xff;
            auto decompressor = Decompressor::create('H', dict);
            verify(decompressor);
        }

        if ((originalValue == 0) || (originalValue == 0xff)) {
            dict[0][i] = originalValue;
            continue;
        }

        {
            dict[0][i] = 0;
            auto decompressor = Decompressor::create('H', dict);
            verify(decompressor);
        }

        {
            dict[0][i] = static_cast<unsigned char>(0xff);
            auto decompressor = Decompressor::create('H', dict);
            verify(decompressor);
        }

        dict[0][i] = originalValue;
    }
}

QTEST_GUILESS_MAIN(DecompressorTest)

#include "decompressortest.moc"
