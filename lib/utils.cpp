// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "utils.h"

#include <QIODevice>

bool Utils::readUint8(QIODevice *device, quint8 &value)
{
    unsigned char word[1] = {0};
    if (!device->read((char *)&word, 1)) {
        return false;
    }
    value = (quint8)word[0];
    return true;
}

bool Utils::readUint16(QIODevice *device, quint16 &value)
{
    unsigned char word[2] = {0, 0};
    if (!device->read((char *)&word, 2)) {
        return false;
    }

    value = (quint16)((quint16)word[0] << 8 | (quint16)word[1]);
    return true;
}

bool Utils::readUint32(QIODevice *device, quint32 &value)
{
    unsigned char word[4] = {0, 0, 0, 0};
    if (!device->read((char *)&word, 4)) {
        return false;
    }

    value = (quint16)((quint16)word[0] << 24 | (quint16)word[1] << 16 | (quint16)word[2] << 8 | (quint16)word[3]);
    return true;
}
