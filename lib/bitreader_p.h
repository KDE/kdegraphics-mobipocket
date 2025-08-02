// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// Huffdic decompressor based on Python code by Igor Skochinsky
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QByteArray>
#include <QtEndian>

namespace Mobipocket
{
class BitReader
{
public:
    BitReader(QByteArrayView d)
        : len(d.size() * 8)
        , data(d)
    {
    }

    quint32 read()
    {
        while (rEndPos - pos < 32) {
            // r does not hold sufficient data, fetch some more
            qint64 bytePos = rEndPos / 8;

            if (len - rEndPos >= 32) {
                r <<= 32;
                r |= qFromBigEndian<quint32>(data.constData() + bytePos);
                rEndPos += 32;
                break;
            } else if (len - rEndPos > 0) {
                r <<= 8;
                quint8 d = data.at(bytePos);
                r |= d;
                rEndPos += 8;
            } else {
                r <<= 8;
                rEndPos += 8;
            }
        }

        quint64 t = r << (64 - (rEndPos - pos));
        return t >> 32;
    }

    bool eat(int n)
    {
        pos += n;
        return pos <= len;
    }

    int left()
    {
        return len - pos;
    }

private:
    quint64 r = 0;
    int pos = 0;
    int len = 0;
    int rEndPos = 0; //< position past the LSB of r
    QByteArrayView data;
};
} // namespace Mobipocket
