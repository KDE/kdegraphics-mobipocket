// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MOBI_DECOMPRESSOR_H
#define MOBI_DECOMPRESSOR_H

#include <QByteArray>
#include <memory>

class KPDBFile;

namespace Mobipocket
{

class Decompressor
{
public:
    Decompressor(const KPDBFile &p)
        : pdb(p)
        , valid(true)
    {
    }
    virtual QByteArray decompress(const QByteArray &data) = 0;
    virtual ~Decompressor()
    {
    }
    bool isValid() const
    {
        return valid;
    }

    static std::unique_ptr<Decompressor> create(quint8 type, const KPDBFile &pdbFile);

protected:
    const KPDBFile &pdb;
    bool valid;
};

quint32 readBELong(const QByteArray &data, int offset);
}
#endif
