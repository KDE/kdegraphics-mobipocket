// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "qmobipocket_private_export.h"

#include <QDateTime>
#include <QSharedDataPointer>
#include <QString>
#include <memory>

class QIODevice;

struct KPDBFilePrivate;
struct KPDBHeaderPrivate;
class KPDBFile;

/**
 * This class contains the information of the PDB file header.
 */
class QMOBIPOCKET_TESTS_EXPORT KPDBHeader final
{
public:
    /// Constructor
    explicit KPDBHeader();
    KPDBHeader(KPDBHeader &&) noexcept;
    KPDBHeader(const KPDBHeader &) noexcept;
    KPDBHeader &operator=(KPDBHeader &&) noexcept;
    KPDBHeader &operator=(const KPDBHeader &);
    ~KPDBHeader();

    /// Get the database name of the file.
    QByteArray name() const;
    void setName(const QByteArray &name);

    /// Get the attributes bitfield of the file.
    quint16 attributes() const;
    void setAttributes(quint16 attributes);

    /// Get the file version of the file.
    quint16 version() const;
    void setVersion(quint16 version);

    /// Get the creation time of the file.
    QDateTime creationTime() const;
    void setCreationTime(const QDateTime &creationTime);

    /// Get the modification time of the file.
    QDateTime modificationTime() const;
    void setModificationTime(const QDateTime &modificationTime);

    /// Get the backup time of the file.
    QDateTime backupTime() const;
    void setBackupTime(const QDateTime &backupTime);

    /// Get the modification number of the file.
    quint32 modificationNumber() const;
    void setModificationNumber(quint32 modificationNumber);

    /// Get the offset to the application info.
    quint32 appInfoOffset() const;
    void setAppInfoOffset(quint32 appInfoOffset);

    /// Get the offset to the sort info.
    quint32 sortInfoOffset() const;
    void setSortInfoOffset(quint32 sortInfoOffset);

    /// Get the database type.
    QByteArray databaseType() const;
    void setDatabaseType(const QByteArray &databaseType);

    /// Get the creator type.
    QByteArray creator() const;
    void setCreator(const QByteArray &creator);

    /// Get the internal uid.
    quint32 uid() const;
    void setUid(quint32 uid);

    /// Not used
    quint32 nextRecord() const;
    void setNextRecord(quint32 nextRecord);

    /// Get the number of records in the file.
    quint16 recordCount() const;
    void setRecordCount(quint32 recordCount);

private:
    friend KPDBFile;
    friend KPDBFilePrivate;
    QExplicitlySharedDataPointer<KPDBHeaderPrivate> d;
};

/**
 * This class represents a PDB file.
 *
 * See https://en.wikipedia.org/wiki/PDB_(Palm_OS)
 */
class QMOBIPOCKET_TESTS_EXPORT KPDBFile final
{
public:
    /*!
      Create an empty KDBFile
    */
    KPDBFile();

    /*!
       Create a KDBFile from an IO device.
     */
    explicit KPDBFile(QIODevice &device);

    ~KPDBFile();

    [[nodiscard]] bool isValid() const;

    [[nodiscard]] KPDBHeader header() const;
    void setHeader(const KPDBHeader &header);

    [[nodiscard]] QByteArray recordAt(int record) const;

    void addRecord(const QByteArray &record, quint8 attributes = 0);

    void write(QIODevice &device);

private:
    std::unique_ptr<KPDBFilePrivate> const d;
};
