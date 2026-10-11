# SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

qt_add_executable(bigscreen-music
    main.cpp

    library/artworkcache.cpp
    library/artworkcache.h
    library/tagreader.cpp
    library/tagreader.h
    library/track.h

    playback/mpris.cpp
    playback/mpris.h
    playback/player.cpp
    playback/player.h
    playback/playqueue.cpp
    playback/playqueue.h
)

ecm_add_qml_module(bigscreen-music
    URI org.kde.bigscreen.music
    GENERATE_PLUGIN_SOURCE
    NO_IMPORT_SCAN
    DEPENDENCIES
        QtQuick
        QtMultimedia
        org.kde.bigscreen.ui
        org.kde.kirigami
)

ecm_target_qml_sources(bigscreen-music SOURCES
    qml/Main.qml
    qml/NowPlayingButton.qml
    qml/NowPlayingPage.qml
    qml/PlaceholderPage.qml
    qml/PlayingIndicator.qml
    qml/SettingsPage.qml
    qml/Shell.qml
)

target_include_directories(bigscreen-music PRIVATE
    ${PROJECT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/library
    ${CMAKE_CURRENT_SOURCE_DIR}/playback
)

target_link_libraries(bigscreen-music PRIVATE
    Qt6::Concurrent
    Qt6::DBus
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Qml
    Qt6::Quick
    KF6::CoreAddons
    KF6::Crash
    KF6::DBusAddons
    KF6::FileMetaData
    KF6::I18n
    KF6::I18nQml
)

install(TARGETS bigscreen-music ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
