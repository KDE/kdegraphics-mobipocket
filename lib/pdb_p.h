// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MOBIPOCKET_PDB_P_H
#define MOBIPOCKET_PDB_P_H

#include <QByteArray>

#include <memory>

class QIODevice;

namespace Mobipocket
{
struct PDBPrivate;
class PDB
{
public:
    explicit PDB(QIODevice *device);
    ~PDB();

    QByteArray fileType() const;
    quint16 recordCount() const;
    QByteArray getRecord(quint16 i) const;
    bool isValid() const;

    Q_DISABLE_COPY(PDB);

private:
    std::unique_ptr<PDBPrivate> d;
};
}
#endif
