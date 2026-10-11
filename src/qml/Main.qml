// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.ApplicationWindow {
    id: root

    width: 1920
    height: 1080
    visible: true
    title: i18n("Music")

    function openNowPlaying(): void {
        if (!(stack.currentItem instanceof NowPlayingPage)) {
            stack.push(nowPlayingPage);
        }
    }

    function openSettings(): void {
        if (!(stack.currentItem instanceof SettingsPage)) {
            stack.push(settingsPage);
        }
    }

    BigscreenUI.StackView {
        id: stack
        anchors.fill: parent
        focus: true
        initialItem: Shell {}
    }

    Component {
        id: nowPlayingPage
        NowPlayingPage {}
    }

    Component {
        id: settingsPage
        SettingsPage {}
    }

    Shortcut {
        sequence: "Toggle Media Play/Pause"
        context: Qt.ApplicationShortcut
        onActivated: Player.togglePlayPause()
    }
    Shortcut {
        sequence: "Media Play"
        context: Qt.ApplicationShortcut
        onActivated: Player.play()
    }
    Shortcut {
        sequence: "Media Pause"
        context: Qt.ApplicationShortcut
        onActivated: Player.pause()
    }
    Shortcut {
        sequence: "Media Stop"
        context: Qt.ApplicationShortcut
        onActivated: Player.stop()
    }
    Shortcut {
        sequence: "Media Next"
        context: Qt.ApplicationShortcut
        onActivated: Player.next()
    }
    Shortcut {
        sequence: "Media Previous"
        context: Qt.ApplicationShortcut
        onActivated: Player.previous()
    }
}
