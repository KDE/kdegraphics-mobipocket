/***************************************************************************
 *   Copyright (C) 2008 by Jakub Stachowski <qbast@go2.pl>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "qfilestream.h"

using namespace Mobipocket;

QFileStream::QFileStream(const QString& name)
 : d(new QFile(name))
{
    d->open(QIODevice::ReadOnly);
}

QFileStream::~QFileStream()
{
    delete d;
}

int QFileStream::read(char* buf, int size)
{
    return d->read(buf,size);
}

bool QFileStream::seek(int pos)
{
    return d->seek(pos);
}
