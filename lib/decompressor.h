// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MOBI_DECOMPRESSOR_H
#define MOBI_DECOMPRESSOR_H

#include <QByteArray>
#include <memory>
namespace Mobipocket
{

class Decompressor
{
public:
    Decompressor() = default;
    virtual ~Decompressor() = default;
    virtual QByteArray decompress(const QByteArray &data) = 0;
    bool isValid() const
    {
        return valid;
    }

    static std::unique_ptr<Decompressor> create(quint8 type, const QVector<QByteArray> &auxData);

protected:
    bool valid = false;
};
}
#endif
