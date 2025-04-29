// SPDX-FileCopyrightText: 2008 by Jakub Stachowski <qbast@go2.pl>
// SPDX-License-Identifier: GPL-2.0-or-later

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
