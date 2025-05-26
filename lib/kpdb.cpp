// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kpdb_p.h"
#include "utils.h"

#include <QIODevice>
#include <QTimeZone>

namespace
{
constexpr auto PALMDB_NAME_SIZE_MAX = 32;
constexpr auto PALMDB_RECORD_INFO_SIZE = 8;

/// Difference in seconds between epoch time and mac time */
constexpr auto EPOCH_MAC_DIFF = 2082844800UL;

/// Swap endianness of 32-bit value
quint32 swap32(const quint32 val)
{
    return ((((val) >> 24) & 0x000000ff) | (((val) >> 8) & 0x0000ff00) | (((val) << 8) & 0x00ff0000) | (((val) << 24) & 0xff000000));
}

QDateTime fromPdbtime(const long pdb_time)
{
    time_t time = pdb_time;
    const int unix1996 = 820454400;
    if (time < unix1996 && time > 0) {
        /* sometimes dates are stored as little endian */
        time = swap32((uint32_t)time);
    }
    const quint32 mactime_flag = (quint32)(1U << 31);
    if (time & mactime_flag) {
        time -= EPOCH_MAC_DIFF;
    }
    return QDateTime::fromSecsSinceEpoch(time, QTimeZone::UTC);
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
    void loadMetadata(QIODevice *device);
    void loadRecordList(QIODevice *device);
    void loadRecords(QIODevice *device);
};

void KPDBFile::read(QIODevice *device)
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

void KPDBFilePrivate::loadMetadata(QIODevice *device)
{
    if (!device->read(header.d->name.data(), PALMDB_NAME_SIZE_MAX)) {
        qWarning() << "failure reading name";
        valid = false;
        return;
    }

    if (!Utils::readUint16(device, header.d->attributes)) {
        valid = false;
        return;
    }

    if (!Utils::readUint16(device, header.d->version)) {
        valid = false;
        return;
    }

    if (!Utils::readUint32(device, header.d->ctime)) {
        valid = false;
        return;
    }

    if (!Utils::readUint32(device, header.d->mtime)) {
        return;
    }

    if (!Utils::readUint32(device, header.d->btime)) {
        return;
    }

    if (!Utils::readUint32(device, header.d->modificationNumber)) {
        return;
    }

    if (!Utils::readUint32(device, header.d->appInfoOffset)) {
        return;
    }

    if (!Utils::readUint32(device, header.d->sortInfoOffset)) {
        return;
    }

    if (!device->read(header.d->type.data(), 4)) {
        valid = false;
        return;
    }

    if (!device->read(header.d->creator.data(), 4)) {
        valid = false;
        return;
    }

    if (!Utils::readUint32(device, header.d->uid)) {
        return;
    }

    if (!Utils::readUint32(device, header.d->nextRecord)) {
        return;
    }

    if (!Utils::readUint16(device, header.d->recordCount)) {
        return;
    }
}

void KPDBFilePrivate::loadRecordList(QIODevice *device)
{
    for (int i = 0; i < header.recordCount(); i++) {
        KPDBRecord record;

        if (!Utils::readUint32(device, record.offset)) {
            return;
        }

        if (!Utils::readUint8(device, record.attributes)) {
            return;
        }

        quint8 h;
        if (!Utils::readUint8(device, h)) {
            return;
        }

        quint16 l;
        if (!Utils::readUint16(device, l)) {
            return;
        }

        record.uid = (quint32)h << 16 | l;
        records << record;
    }
}

void KPDBFilePrivate::loadRecords(QIODevice *device)
{
    for (int i = 0, count = records.size(); i < count; i++) {
        KPDBRecord &curr = records[i];
        KPDBRecord next;

        const quint32 offset = curr.offset;
        const bool last = (i == (count - 1));
        if (!device->seek(offset)) {
            continue;
        }
        if (last) {
            curr.data = device->readAll();
            return;
        }
        curr.data = device->read(records[i + 1].offset - offset);
    }
}

KPDBFile::KPDBFile()
    : d(std::make_unique<KPDBFilePrivate>())
{
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
