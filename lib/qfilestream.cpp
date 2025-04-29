// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qfilestream.h"

using namespace Mobipocket;

QFileStream::QFileStream(const QString &name)
    : d(new QFile(name))
{
    d->open(QIODevice::ReadOnly);
}

QFileStream::~QFileStream()
{
    delete d;
}

int QFileStream::read(char *buf, int size)
{
    return d->read(buf, size);
}

bool QFileStream::seek(int pos)
{
    return d->seek(pos);
}
