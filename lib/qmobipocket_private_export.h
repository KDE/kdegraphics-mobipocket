// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "qmobipocket_export.h"

/* Classes which are exported only for unit tests */
#ifdef BUILD_TESTING
#ifndef QMOBIPOCKET_TESTS_EXPORT
#define QMOBIPOCKET_TESTS_EXPORT QMOBIPOCKET_EXPORT
#endif
#else /* not compiling tests */
#define QMOBIPOCKET_TESTS_EXPORT
#endif
