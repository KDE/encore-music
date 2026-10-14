// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

RowLayout {
    id: root

    property string title
    property string summary

    spacing: Kirigami.Units.gridUnit

    BigscreenUI.Label {
        text: root.title
        type: BigscreenUI.Label.Title
        Accessible.role: Accessible.Heading
    }
    BigscreenUI.Label {
        Layout.alignment: Qt.AlignBaseline
        visible: text.length > 0
        text: root.summary
        color: Kirigami.Theme.disabledTextColor
        type: BigscreenUI.Label.Heading
    }
}
