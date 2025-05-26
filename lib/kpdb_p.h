// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "qmobipocket_private_export.h"

#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QString>

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
    ~KPDBHeader();

    /// Get the database name of the file.
    QByteArray name() const;

    /// Get the attributes bitfield of the file.
    quint16 attributes() const;

    /// Get the file version of the file.
    quint16 version() const;

    /// Get the creation time of the file.
    QDateTime creationTime() const;

    /// Get the modification time of the file.
    QDateTime modificationTime() const;

    /// Get the backup time of the file.
    QDateTime backupTime() const;

    /// Get the modification number of the file.
    quint32 modificationNumber() const;

    /// Get the offset to the application info.
    quint32 appInfoOffset() const;

    /// Get the offset to the sort info.
    quint32 sortInfoOffset() const;

    /// Get the database type.
    QByteArray databaseType() const;

    /// Get the creator type.
    QByteArray creator() const;

    /// Get the internal uid.
    quint32 uid() const;

    /// Not used
    quint32 nextRecord() const;

    /// Get the number of records in the file.
    quint16 recordCount() const;

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
    KPDBFile();
    ~KPDBFile();

    [[nodiscard]] bool isValid() const;

    [[nodiscard]] KPDBHeader header() const;
    [[nodiscard]] QByteArray recordAt(int record) const;

    void read(QIODevice *device);

private:
    std::unique_ptr<KPDBFilePrivate> const d;
};
