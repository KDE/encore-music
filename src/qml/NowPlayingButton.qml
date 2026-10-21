// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

T.AbstractButton {
    id: root

    visible: Player.hasTrack

    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    implicitWidth: implicitContentWidth + leftPadding + rightPadding

    padding: Kirigami.Units.smallSpacing * 2
    rightPadding: Kirigami.Units.gridUnit

    focusPolicy: Qt.StrongFocus
    hoverEnabled: true

    Accessible.name: i18nc("@action:button %1 is a song title, %2 its artist", "Now playing: %1 by %2", Player.currentTrack.title, Player.currentTrack.artist)

    onPressed: root.forceActiveFocus()
    BigscreenUI.EnterKeyHandler.enabled: true

    background: BigscreenUI.DelegateBackground {
        control: root
        raisedBackground: false
    }

    contentItem: RowLayout {
        spacing: 0

        Kirigami.ShadowedImage {
            Layout.fillHeight: true
            Layout.preferredWidth: height

            source: Player.currentTrack.artworkUrl
            sourceSize.width: width * Screen.devicePixelRatio
            sourceSize.height: height * Screen.devicePixelRatio
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            radius: Kirigami.Units.cornerRadius
            color: Kirigami.Theme.alternateBackgroundColor

            Kirigami.Icon {
                anchors.centerIn: parent
                width: Kirigami.Units.iconSizes.small
                height: width
                visible: parent.status !== Image.Ready
                source: "media-album-cover"
                opacity: 0.5
            }
        }

        ColumnLayout {
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.maximumWidth: Kirigami.Units.gridUnit * 14
            spacing: 0

            BigscreenUI.Label {
                Layout.fillWidth: true
                text: Player.currentTrack.title
                elide: Text.ElideRight
                Accessible.ignored: true
            }
            BigscreenUI.Label {
                Layout.fillWidth: true
                visible: text.length > 0
                text: Player.currentTrack.artist
                type: BigscreenUI.Label.Small
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
                Accessible.ignored: true
            }
        }

        PlayingIndicator {
            Layout.leftMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: implicitHeight
            playing: Player.playing
        }
    }
}
