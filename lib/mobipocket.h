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

/**
Minimalistic stream abstraction. It is supposed to allow mobipocket document classes to be
used with QIODevice (for Okular generator), and previously also with InputStream for Strigi
analyzer.
*/
class QMOBIPOCKET_EXPORT Stream
{
public:
    virtual int read(char *buf, int size) = 0;
    virtual bool seek(int pos) = 0;

    QByteArray readAll();
    QByteArray read(int len);
    virtual ~Stream()
    {
    }
};

struct PDBPrivate;
class PDB
{
public:
    PDB(Stream *s);
    ~PDB();
    QString fileType() const;
    int recordCount() const;
    QByteArray getRecord(int i) const;
    bool isValid() const;

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
    ~Document();
    Document(Stream *s);
    QMap<MetaKey, QString> metadata() const;
    QString text(int size = -1) const;
    int imageCount() const;
    QImage getImage(int i) const;
    QImage thumbnail() const;
    bool isValid() const;

    // if true then it is impossible to get text of book. Images should still be readable
    bool hasDRM() const;

private:
    DocumentPrivate *const d;
};
}
#endif
