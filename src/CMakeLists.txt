# SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

qt_add_executable(bigscreen-music
    main.cpp
)

ecm_add_qml_module(bigscreen-music
    URI org.kde.bigscreen.music
    GENERATE_PLUGIN_SOURCE
    NO_IMPORT_SCAN
    DEPENDENCIES
        QtQuick
        org.kde.bigscreen.ui
        org.kde.kirigami
)

ecm_target_qml_sources(bigscreen-music SOURCES
    qml/Main.qml
)

target_link_libraries(bigscreen-music PRIVATE
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    KF6::CoreAddons
    KF6::Crash
    KF6::DBusAddons
    KF6::I18n
    KF6::I18nQml
)

target_include_directories(bigscreen-music PRIVATE ${PROJECT_BINARY_DIR})

install(TARGETS bigscreen-music ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
