// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QByteArray>

namespace Mobipocket
{

/// See https://wiki.mobileread.com/wiki/MOBI#PalmDOC_Header
struct PalmDocHeader {
    quint16 compression; /**< 0 */
    // padding 2 bytes
    quint32 textLength; /**< 4 */
    quint16 recordCount; /**< 8 */
    quint16 recordSize; /**< 10 */
    quint16 encryptionType; /**< 12 */
    // padding 2 bytes
};
}
