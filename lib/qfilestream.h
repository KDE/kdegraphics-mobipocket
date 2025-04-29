/***************************************************************************
 *   Copyright (C) 2008 by Jakub Stachowski <qbast@go2.pl>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef QFILESTREAM_H
#define QFILESTREAM_H

#include "mobipocket.h"
#include <QFile>

namespace Mobipocket
{

class QMOBIPOCKET_EXPORT QFileStream : public Stream
{
public:
    QFileStream(const QString &name);
    ~QFileStream();
    int read(char *buf, int size) override;
    bool seek(int pos) override;

private:
    QFile *d;
};

}
#endif
