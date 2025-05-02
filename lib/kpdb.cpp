// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kpdb_p.h"

#include <QDebug>
#include <QIODevice>
#include <QSharedData>
#include <QTimeZone>
#include <QtEndian>

namespace
{
constexpr auto PALMDB_NAME_SIZE_MAX = 32;

bool readUint8(QIODevice &device, quint8 &value)
{
    unsigned char word[1] = {0};
    if (!device.read((char *)&word, 1)) {
        return false;
    }
    value = qFromBigEndian<quint8>(word);
    return true;
}

bool readUint16(QIODevice &device, quint16 &value)
{
    unsigned char word[2] = {0, 0};
    if (!device.read((char *)&word, 2)) {
        return false;
    }

    value = qFromBigEndian<quint16>(word);
    return true;
}

bool readUint32(QIODevice &device, quint32 &value)
{
    unsigned char word[4] = {0, 0, 0, 0};
    if (!device.read((char *)&word, 4)) {
        return false;
    }

    value = qFromBigEndian<quint32>(word);
    return true;
}

/// Difference in seconds between epoch time and mac time */
constexpr auto EPOCH_MAC_DIFF = 2082844800UL;

QDateTime fromPdbtime(quint32 pdbTime)
{
    time_t time = pdbTime;
    const int unix1996 = 820454400;
    if (time < unix1996 && time > 0) {
        /* sometimes dates are stored as little endian */
        time = qFromLittleEndian<quint32>(time);
    }
    const quint32 mactime_flag = (quint32)(1U << 31);
    if (time & mactime_flag) {
        time -= EPOCH_MAC_DIFF;
    }
    return QDateTime::fromSecsSinceEpoch(time, QTimeZone::utc());
}

}

struct KPDBRecord {
    quint32 offset; // Record
    size_t size; ///< Calculated size of the record data
    quint8 attributes; ///< Record attributes
    quint32 uid; ///< Record unique id, usually sequential even numbers
    QByteArray data; ///< Record data
};

struct KPDBHeaderPrivate : public QSharedData {
    QByteArray name; ///< offset 0
    quint16 attributes; ///< offset 32
    quint16 version; ///< offset 34
    quint32 ctime; ///< offset 36
    quint32 mtime; ///< offset 40
    quint32 btime; ///< offset 44
    quint32 modificationNumber; ///< offset 48
    quint32 appInfoOffset; ///< offset 52
    quint32 sortInfoOffset; ///< offset 56
    QByteArray type; ///< offset 60
    QByteArray creator; ///< offset 64
    quint32 uid; ///< offset 68
    quint32 nextRecord; ///< offset 72
    quint16 recordCount; ///< offset 76
};

KPDBHeader::KPDBHeader()
    : d(new KPDBHeaderPrivate)
{
}

KPDBHeader::KPDBHeader(KPDBHeader &&) noexcept = default;
KPDBHeader::KPDBHeader(const KPDBHeader &) noexcept = default;
KPDBHeader &KPDBHeader::operator=(KPDBHeader &&) noexcept = default;
KPDBHeader &KPDBHeader::operator=(const KPDBHeader &) = default;
KPDBHeader::~KPDBHeader() = default;

QByteArray KPDBHeader::name() const
{
    return d->name;
}

quint16 KPDBHeader::attributes() const
{
    return d->attributes;
}

quint16 KPDBHeader::version() const
{
    return d->version;
}

QDateTime KPDBHeader::creationTime() const
{
    return fromPdbtime(d->ctime);
}

QDateTime KPDBHeader::modificationTime() const
{
    return fromPdbtime(d->mtime);
}

QDateTime KPDBHeader::backupTime() const
{
    return fromPdbtime(d->btime);
}

quint32 KPDBHeader::modificationNumber() const
{
    return d->modificationNumber;
}

quint32 KPDBHeader::appInfoOffset() const
{
    return d->appInfoOffset;
}

quint32 KPDBHeader::sortInfoOffset() const
{
    return d->sortInfoOffset;
}

QByteArray KPDBHeader::databaseType() const
{
    return d->type;
}

QByteArray KPDBHeader::creator() const
{
    return d->creator;
}

quint32 KPDBHeader::uid() const
{
    return d->uid;
}

quint32 KPDBHeader::nextRecord() const
{
    return d->nextRecord;
}

quint16 KPDBHeader::recordCount() const
{
    return d->recordCount;
}

struct KPDBFilePrivate : public QSharedData {
    bool valid = true;
    KPDBHeader header;
    QList<KPDBRecord> records;

    void init();
    bool loadMetadata(QIODevice &device);
    bool loadRecordList(QIODevice &device);
    bool loadRecords(QIODevice &device);
};

bool KPDBFilePrivate::loadMetadata(QIODevice &device)
{
    if (!device.read(header.d->name.data(), PALMDB_NAME_SIZE_MAX)) {
        qWarning() << "failure reading name";
        return false;
    }

    if (!readUint16(device, header.d->attributes)) {
        return false;
    }

    if (!readUint16(device, header.d->version)) {
        return false;
    }

    if (!readUint32(device, header.d->ctime)) {
        return false;
    }

    if (!readUint32(device, header.d->mtime)) {
        return false;
    }

    if (!readUint32(device, header.d->btime)) {
        return false;
    }

    if (!readUint32(device, header.d->modificationNumber)) {
        return false;
    }

    if (!readUint32(device, header.d->appInfoOffset)) {
        return false;
    }

    if (!readUint32(device, header.d->sortInfoOffset)) {
        return false;
    }

    if (!device.read(header.d->type.data(), 4)) {
        return false;
    }

    if (!device.read(header.d->creator.data(), 4)) {
        return false;
    }

    if (!readUint32(device, header.d->uid)) {
        return false;
    }

    if (!readUint32(device, header.d->nextRecord)) {
        return false;
    }

    if (!readUint16(device, header.d->recordCount)) {
        return false;
    }

    return true;
}

bool KPDBFilePrivate::loadRecordList(QIODevice &device)
{
    for (int i = 0; i < header.recordCount(); i++) {
        KPDBRecord record;

        if (!readUint32(device, record.offset)) {
            return false;
        }

        if (!readUint8(device, record.attributes)) {
            return false;
        }

        quint8 h;
        if (!readUint8(device, h)) {
            return false;
        }

        quint16 l;
        if (!readUint16(device, l)) {
            return false;
        }

        record.uid = (quint32)h << 16 | l;
        records << record;
    }
    return true;
}

bool KPDBFilePrivate::loadRecords(QIODevice &device)
{
    for (int i = 0, count = records.size(); i < count; i++) {
        KPDBRecord &curr = records[i];
        KPDBRecord next;

        const quint32 offset = curr.offset;
        const bool last = (i == (count - 1));
        if (!device.seek(offset)) {
            continue;
        }
        if (last) {
            curr.data = device.readAll();
            return true;
        }
        const auto size = records[i + 1].offset - offset;
        curr.data = device.read(size);
        if (curr.data.size() != size) {
            return false;
        }
    }

    Q_UNREACHABLE();
    return true;
}

KPDBFile::KPDBFile()
    : d(std::make_unique<KPDBFilePrivate>())
{
}

KPDBFile::KPDBFile(QIODevice &device)
    : d(std::make_unique<KPDBFilePrivate>())
{
    d->header.d->name.resize(PALMDB_NAME_SIZE_MAX);
    d->header.d->type.resize(4);
    d->header.d->creator.resize(4);

    d->loadMetadata(device);
    if (d->header.recordCount() == 0) {
        qWarning() << "No records found";
        d->valid = false;
        return;
    }

    d->loadRecordList(device);

    if (!d->valid) {
        return;
    }

    d->loadRecords(device);

    if (!d->valid) {
        return;
    }
}

KPDBFile::~KPDBFile() = default;

bool KPDBFile::isValid() const
{
    return d->valid;
}

QByteArray KPDBFile::recordAt(int record) const
{
    if (record >= d->records.size()) {
        qWarning() << "trying to access record" << record << "but only" << d->records.size() << "exists";
        return {};
    }
    return d->records.at(record).data;
}

KPDBHeader KPDBFile::header() const
{
    return d->header;
}
