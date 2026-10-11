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

    BigscreenUI.StackView {
        id: stack
        anchors.fill: parent
        focus: true

        initialItem: BigscreenUI.ScrollablePage {
            title: i18n("Music")
        }
    }
}
