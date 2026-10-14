// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.Page {
    id: root

    property alias iconName: message.iconName
    property alias text: message.text
    property alias explanation: message.explanation
    property alias helpfulAction: message.helpfulAction

    BigscreenUI.PlaceholderMessage {
        id: message
        anchors.centerIn: parent
        width: Math.min(parent.width - root.contentMargins.left - root.contentMargins.right, implicitWidth)
        focus: true
    }
}
