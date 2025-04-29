// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MOBIPOCKET_H
#define MOBIPOCKET_H

#include <QByteArray>
#include <QImage>
#include <QMap>
#include <QString>

#include "qmobipocket_export.h"

class QIODevice;

namespace Mobipocket
{

struct PDBPrivate;
class PDB
{
public:
    explicit PDB(QIODevice *device);
    ~PDB();
    QString fileType() const;
    int recordCount() const;
    QByteArray getRecord(int i) const;
    bool isValid() const;

    Q_DISABLE_COPY(PDB);
private:
    PDBPrivate *const d;
};

struct DocumentPrivate;
class QMOBIPOCKET_EXPORT Document
{
public:
    enum MetaKey {
        Title,
        Author,
        Copyright,
        Description,
        Subject
    };

    /**
     * Mobipocket::Document constructor
     *
     * @params device The IO device corresponding to the mobipocket document. The device must
     * be open for read operations, not sequential and the document does not take ownership of
     * the device.
     */
    explicit Document(QIODevice *device);
    virtual ~Document();

    QMap<MetaKey, QString> metadata() const;
    QString text(int size=-1) const;
    int imageCount() const;
    QImage getImage(int i) const;
    QImage thumbnail() const;
    bool isValid() const;

    // if true then it is impossible to get text of book. Images should still be readable
    bool hasDRM() const;

    Q_DISABLE_COPY(Document);
private:
    DocumentPrivate *const d;
};
}
#endif
