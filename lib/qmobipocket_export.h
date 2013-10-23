/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _QMOBIPOCKET_EXPORT_H
#define _QMOBIPOCKET_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef QMOBIPOCKET_EXPORT
# if defined(KDELIBS_STATIC_LIBS)
   /* No export/import for static libraries */
#  define QMOBIPOCKET_EXPORT
# elif defined(MAKE_QMOBIPOCKET_LIB)
   /* We are building this library */
#  define QMOBIPOCKET_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define QMOBIPOCKET_EXPORT KDE_IMPORT
# endif
#endif

#endif
