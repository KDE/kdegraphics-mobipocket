// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// RLE decompressor based on FBReader
// SPDX-FileCopyrightText: 2004-2008 Geometer Plus <contact@geometerplus.com>
// Huffdic decompressor based on Python code by Igor Skochinsky
// SPDX-License-Identifier: GPL-2.0-or-later

#include "decompressor.h"

#include "bitreader_p.h"

#include <QVector>
#include <QtEndian>

// clang-format off
static const unsigned char TOKEN_CODE[256] = {
	0, 1, 1, 1,		1, 1, 1, 1,		1, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	3, 3, 3, 3,		3, 3, 3, 3,		3, 3, 3, 3,		3, 3, 3, 3,
	3, 3, 3, 3,		3, 3, 3, 3,		3, 3, 3, 3,		3, 3, 3, 3,
	3, 3, 3, 3,		3, 3, 3, 3,		3, 3, 3, 3,		3, 3, 3, 3,
	3, 3, 3, 3,		3, 3, 3, 3,		3, 3, 3, 3,		3, 3, 3, 3,
	2, 2, 2, 2,		2, 2, 2, 2,		2, 2, 2, 2,		2, 2, 2, 2,
	2, 2, 2, 2,		2, 2, 2, 2,		2, 2, 2, 2,		2, 2, 2, 2,
	2, 2, 2, 2,		2, 2, 2, 2,		2, 2, 2, 2,		2, 2, 2, 2,
	2, 2, 2, 2,		2, 2, 2, 2,		2, 2, 2, 2,		2, 2, 2, 2,
};
// clang-format on

namespace Mobipocket
{

class NOOPDecompressor : public Decompressor
{
public:
    NOOPDecompressor()
    {
        valid = true;
    }
    QByteArray decompress(const QByteArray &data) override
    {
        return data;
    }
};

class RLEDecompressor : public Decompressor
{
public:
    RLEDecompressor()
    {
        valid = true;
    }
    QByteArray decompress(const QByteArray &data) override;
};

class HuffdicDecompressor : public Decompressor
{
public:
    HuffdicDecompressor() = delete;
    HuffdicDecompressor(const HuffdicDecompressor &) = delete;
    HuffdicDecompressor(const QVector<QByteArray> &huffData);
    QByteArray decompress(const QByteArray &data) override;

private:
    void unpack(BitReader reader, int depth = 0);
    const QVector<QByteArray> dicts;
    quint32 entry_bits;
    quint32 dict1[256];
    quint32 dict2[64];

    QByteArray buf;
};

QByteArray RLEDecompressor::decompress(const QByteArray &data)
{
    QByteArray ret;
    ret.reserve(8192);

    int i = 0;
    int maxIndex = data.size() - 1;

    while (i < data.size()) {
        unsigned char token = data.at(i++);
        switch (TOKEN_CODE[token]) {
        case 0:
            ret.append(token);
            break;
        case 1:
            if ((i + token > maxIndex)) {
                return ret;
            }
            ret.append(data.mid(i, token));
            i += token;
            break;
        case 2:
            ret.append(' ');
            ret.append(token ^ 0x80);
            break;
        case 3:
            {
                if (i > maxIndex) {
                    return ret;
                }
                quint16 N = token << 8;
                N += (unsigned char)data.at(i++);
                quint16 copyLength = (N & 7) + 3;
                quint16 shift = (N & 0x3fff) / 8;
                if ((shift < 1) || (shift > ret.size())) {
                    return ret;
                }
                auto shifted = ret.size() - shift;
                for (auto j = shifted; j < shifted + copyLength; j++) {
                    ret.append(ret.at(j));
                }
            }
            break;
        }
    }
    return ret;
}

HuffdicDecompressor::HuffdicDecompressor(const QVector<QByteArray> &huffData)
    : dicts(huffData.mid(1))
{
    if (dicts.empty())
        return;

    if ((dicts[0].size() < 18) || !dicts[0].startsWith("CDIC"))
        return;

    const QByteArray &huff1 = huffData[0];
    if ((huff1.size() < 24) || !huff1.startsWith("HUFF"))
        return;

    quint32 off1 = qFromBigEndian<quint32>(huff1.constData() + 16);
    quint32 off2 = qFromBigEndian<quint32>(huff1.constData() + 20);
    if (((off1 + 256 * 4) > huff1.size()) || ((off2 + 64 * 4) > huff1.size()))
        return;

    memcpy(dict1, huff1.data() + off1, 256 * 4);
    memcpy(dict2, huff1.data() + off2, 64 * 4);

    entry_bits = qFromBigEndian<quint32>(dicts[0].constData() + 12);

    valid = true;
}

QByteArray HuffdicDecompressor::decompress(const QByteArray &data)
{
    buf.clear();
    unpack(BitReader(data));
    return buf;
}

void HuffdicDecompressor::unpack(BitReader reader, int depth)
{
    if (depth > 32)
        goto fail;
    while (reader.left()) {
        quint32 dw = reader.read();
        quint32 v = dict1[dw >> 24];
        quint8 codelen = v & 0x1F;
        if (!codelen)
            goto fail;
        quint32 code = dw >> (32 - codelen);
        quint32 r = (v >> 8);
        if (!(v & 0x80)) {
            while (code < dict2[(codelen - 1) * 2]) {
                codelen++;
                code = dw >> (32 - codelen);
            }
            r = dict2[(codelen - 1) * 2 + 1];
        }
        r -= code;
        if (!codelen)
            goto fail;
        if (!reader.eat(codelen))
            return;
        quint32 dict_no = r >> entry_bits;
        quint32 off1 = 16 + (r - (dict_no << entry_bits)) * 2;
        const QByteArray &dict = dicts.at(dict_no);
        quint16 off2 = 16 + qFromBigEndian<quint16>(dict.constData() + off1);
        quint16 blen = qFromBigEndian<quint16>(dict.constData() + off2);
        QByteArray slice = dict.mid(off2 + 2, (blen & 0x7fff));
        if (blen & 0x8000)
            buf += slice;
        else
            unpack(BitReader(slice), depth + 1);
    }
    return;
fail:
    valid = false;
}

std::unique_ptr<Decompressor> Decompressor::create(quint8 type, const QVector<QByteArray> &auxData)
{
    switch (type) {
    case 1:
        return std::make_unique<NOOPDecompressor>();
    case 2:
        return std::make_unique<RLEDecompressor>();
    case 'H':
        return std::make_unique<HuffdicDecompressor>(auxData);
    default:
        return nullptr;
    }
}
}
