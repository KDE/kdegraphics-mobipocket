// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QtGlobal>

class QIODevice;

namespace Utils
{
bool readUint8(QIODevice *device, quint8 &value);
bool readUint16(QIODevice *device, quint16 &value);
bool readUint32(QIODevice *device, quint32 &value);
}
