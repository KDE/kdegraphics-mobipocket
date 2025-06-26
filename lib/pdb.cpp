// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pdb_p.h"

#include <QIODevice>
#include <QtEndian>

namespace Mobipocket
{

struct PDBPrivate {
    PDBPrivate(QIODevice *dev);

    QIODevice *device;
    QByteArray fileType;
    QList<quint32> recordOffsets;
    bool valid = false;
};

PDBPrivate::PDBPrivate(QIODevice *dev)
    : device(dev)
{
    const auto pdbHead = device->read(0x4e);
    if (pdbHead.size() < 0x4e)
        return;

    fileType = pdbHead.mid(0x3c, 8);

    const auto nrecords = qFromBigEndian<quint16>(pdbHead.constData() + 0x4c);
    const auto recordData = device->read(8 * nrecords);
    if (recordData.size() < 8 * nrecords)
        return;

    quint32 lastOffset = 0x4d + 8 * nrecords;
    for (int i = 0; i < nrecords; i++) {
        const quint32 offset = qFromBigEndian<quint32>(recordData.constData() + 8 * i);
        if (offset < lastOffset) {
            return;
        }
        if (offset > dev->size()) {
            break;
        }
        recordOffsets.append(offset);
        lastOffset = offset;
    }
    valid = true;
}

PDB::~PDB() = default;

PDB::PDB(QIODevice *device)
    : d(new PDBPrivate(device))
{
}

QByteArray PDB::getRecord(quint16 i) const
{
    if (i >= d->recordOffsets.size()) {
        return QByteArray();
    }

    quint32 offset = d->recordOffsets[i];
    quint32 end = (i + 1 < d->recordOffsets.size()) ? d->recordOffsets[i + 1] : d->device->size();

    if (!d->device->seek(offset))
        return QByteArray();

    return d->device->read(end - offset);
}

QByteArray PDB::fileType() const
{
    return d->fileType;
}

bool PDB::isValid() const
{
    return d->valid;
}

quint16 PDB::recordCount() const
{
    // Range guaranteed by constructor/PDB field size
    return d->recordOffsets.size();
}

}
