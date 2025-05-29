// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// Huffdic decompressor based on Python code by Igor Skochinsky
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QByteArray>

namespace Mobipocket
{
class BitReader
{
public:
    BitReader(const QByteArray &d)
        : pos(0)
        , len(d.size() * 8)
        , data(d)
    {
        data.append(4, '\0');
    }

    quint32 read()
    {
        quint32 g = 0;
        quint64 r = 0;
        while (g < 32) {
            r = (r << 8) | (quint8)data[(pos + g) >> 3];
            g = g + 8 - ((pos + g) & 7);
        }
        return (r >> (g - 32));
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
    int pos;
    int len;
    QByteArray data;
};
} // namespace Mobipocket
