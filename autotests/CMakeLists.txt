# SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
# SPDX-Licence-Identifier: BSD-2-Clauses

add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata")

ecm_add_tests(kpdbtest.cpp metadatatest.cpp
    LINK_LIBRARIES Qt::Test qmobipocket
)