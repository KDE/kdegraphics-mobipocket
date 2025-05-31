# SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
# SPDX-Licence-Identifier: BSD-2-Clause

add_executable(kmobimetadata
    kmobimetadata.cpp
)

target_link_libraries (kmobimetadata PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    qmobipocket
)