// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "qmobipocket_private_export.h"

#include <QByteArray>
#include <optional>

namespace Mobipocket
{

struct QMOBIPOCKET_TESTS_EXPORT MobiHeader {
    enum class MobiType : quint32 {
        Unknown = 0,
        MobiBook = 2,
        PalmDocBook = 3,
        Audio = 4,
        Kindlegen12 = 232,
        KF8 = 248,
        News = 257,
        NewsFed = 258,
        NewsMagazine = 259,
        PICS = 513,
        WORD = 514,
        XSL = 515,
        PPT = 516,
        TEXT = 517,
        HTML = 518,
    };

    /* MOBI header, offset 16 */
    QByteArray mobiMagic; /**< 16: M O B I { 77, 79, 66, 73 } */
    quint32 headerLength = 0; /**< 20: the length of the MOBI header, including the previous 4 bytes */
    MobiType mobiType = MobiType::Unknown; /**< 24: mobipocket file type */
    quint32 textEncoding = 0; /**< 28: 1252 = CP1252, 65001 = UTF-8 */
    quint32 uid; /**< 32: unique id */
    quint32 version; /**< 36: mobipocket format */
    quint32 orthIndex; /**< 40: section number of orthographic meta index. */
    quint32 inflIndex; /**< 44: section number of inflection meta index. */
    quint32 namesIndex; /**< 48: section number of names meta index. */
    quint32 keysIndex; /**< 52: section number of keys meta index. */
    quint32 extra0Index; /**< 56: section number of extra 0 meta index. */
    quint32 extra1Index; /**< 60: section number of extra 1 meta index. */
    quint32 extra2Index; /**< 64: section number of extra 2 meta index. */
    quint32 extra3Index; /**< 68: section number of extra 3 meta index. */
    quint32 extra4Index; /**< 72: section number of extra 4 meta index. */
    quint32 extra5Index; /**< 76: section number of extra 5 meta index. */
    quint32 nonTextIndex; /**< 80: first record number (starting with 0) that's not the book's text */
    quint32 fullNameOffset; /**< 84: offset in record 0 (not from start of file) of the full name of the book */
    quint32 fullNameLength; /**< 88: length of the full name */
    quint32 locale; /**< 92: first byte is main language: 09 = English, next byte is dialect, 08 = British, 04 = US */
    quint32 dictInputLang; /**< 96: input language for a dictionary */
    quint32 dictOutputLang; /**< 100: output language for a dictionary */
    quint32 minVersion; /**< 104: minimum mobipocket version support needed to read this file. */
    quint32 imageIndex; /**< 108: first record number (starting with 0) that contains an image (sequential) */
    quint32 huffRecIndex; /**< 112: first huffman compression record */
    quint32 huffRecCount; /**< 116: huffman compression records count */
    quint32 datpRecIndex; /**< 120: section number of DATP record */
    quint32 datpRecCount; /**< 124: DATP records count */
    quint32 exthFlags; /**< 128: bitfield. if bit 6 (0x40) is set, then there's an EXTH record */
    /* 32 unknown bytes, usually 0, related to encryption and unknown6 */
    /* unknown2 */
    /* unknown3 */
    /* unknown4 */
    /* unknown5 */
    quint32 unknown6; /**< 164: use MOBI_NOTSET , related to encryption*/
    quint32 drmOffset; /**< 168: offset to DRM key info in DRMed files. MOBI_NOTSET if no DRM */
    quint32 drmCount; /**< 172: number of entries in DRM info */
    quint32 drmSize; /**< 176: number of bytes in DRM info */
    quint32 drmFlags; /**< 180: some flags concerning DRM info, bit 0 set if password encryption */
    /* 8 unknown bytes 0? */
    /* unknown7 */
    /* unknown8 */
    std::optional<quint16> firstTextIndex; /**< 192: section number of first text record */
    std::optional<quint16> lastTextIndex; /**< 194: */
    std::optional<quint32> fdstIndex; /**< 192 (KF8) section number of FDST record */
    // std::optional<quint32> unknown9; /**< 196: */
    quint32 fdstSectionCount; /**< 196 (KF8) */
    quint32 fcisIndex; /**< 200: section number of FCIS record */
    quint32 fcisCount; /**< 204: FCIS records count */
    quint32 flisIndex; /**< 208: section number of FLIS record */
    quint32 flisCount; /**< 212: FLIS records count */
    quint32 unknown10; /**< 216: */
    quint32 unknown11; /**< 220: */
    quint32 srcsIndex; /**< 224: section number of SRCS record */
    quint32 srcsCount; /**< 228: SRCS records count */
    quint32 unknown12; /**< 232: */
    quint32 unknown13; /**< 236: */
    /* quint16 fill 0 */
    quint16 extraFlags; /**< 242: extra flags */
    quint32 ncxIndex; /**< 244: section number of NCX record  */
    std::optional<quint32> unknown14; /**< 248: */
    std::optional<quint32> fragmentIndex; /**< 248 (KF8) section number of fragments record */
    std::optional<quint32> unknown15; /**< 252: */
    std::optional<quint32> skeletonIndex; /**< 252 (KF8) section number of SKEL record */
    quint32 datpIndex; /**< 256: section number of DATP record */
    std::optional<quint32> unknown16; /**< 260: */
    std::optional<quint32> guideIndex; /**< 260 (KF8) section number of guide record */
    quint32 unknown17; /**< 264: */
    quint32 unknown18; /**< 268: */
    quint32 unknown19; /**< 272: */
    quint32 unknown20; /**< 276: */
};
}
