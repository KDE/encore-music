// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

FocusScope {
    id: root

    property string title
    property string iconName
    property string text
    property string explanation

    BigscreenUI.Label {
        x: BigscreenUI.Units.contentMargin(root.SafeArea.margins.left)
        y: root.SafeArea.margins.top + Kirigami.Units.gridUnit * 2
        text: root.title
        type: BigscreenUI.Label.Title
        Accessible.role: Accessible.Heading
    }

    ColumnLayout {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: root.SafeArea.margins.top / 2
        width: Math.min(parent.width - BigscreenUI.Units.screenMargin * 2, Kirigami.Units.gridUnit * 40)
        spacing: Kirigami.Units.largeSpacing * 2

        Kirigami.Icon {
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: Kirigami.Units.iconSizes.enormous
            implicitHeight: Kirigami.Units.iconSizes.enormous
            source: root.iconName
            opacity: 0.5
        }

        BigscreenUI.Label {
            Layout.fillWidth: true
            text: root.text
            type: BigscreenUI.Label.Section
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }

        BigscreenUI.Label {
            Layout.fillWidth: true
            text: root.explanation
            color: Kirigami.Theme.disabledTextColor
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }
    }
}
