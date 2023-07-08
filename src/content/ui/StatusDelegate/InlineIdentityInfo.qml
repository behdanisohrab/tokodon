// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kmasto 1.0

RowLayout {
    id: root

    required property var identity
    required property bool secondary
    property bool admin: false
    property string ip

    Layout.fillWidth: true
    spacing: Kirigami.Units.largeSpacing

    signal clicked()

    Kirigami.Avatar {
        Layout.alignment: admin ? Qt.AlignCenter : Qt.AlignTop
        Layout.rowSpan: 5
        source: root.identity.avatarUrl
        cache: true
        actions.main: Kirigami.Action {
            tooltip: i18n("View profile")
            onTriggered: {
                if (!admin) {
                    Navigation.openAccount(root.identity.id)
                    root.clicked()
                }
            }
        }
        name: root.identity.displayName
    }

    ColumnLayout {
        id: layout

        Layout.fillWidth: true
        Layout.bottomMargin: Kirigami.Units.smallSpacing

        spacing: 0
        clip: true

        Kirigami.Heading {
            level: 4
            font.pixelSize: Config.defaultFont.pixelSize + 2
            font.pointSize: -1
            text: root.identity.displayNameHtml
            type: Kirigami.Heading.Type.Primary
            color: root.secondary ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
            verticalAlignment: Text.AlignTop
            elide: Text.ElideRight
            textFormat: Text.RichText
            Layout.fillWidth: true
        }

        QQC2.Label {
            font.pixelSize: Config.defaultFont.pixelSize + 1
            Layout.fillWidth: true
            elide: Text.ElideRight
            color: Kirigami.Theme.disabledTextColor
            text: `@${root.identity.account}`
            verticalAlignment: Text.AlignTop
        }
        Kirigami.Heading {
            id: emailHeading
            level: 4
            text: root.ip ? root.ip : ""
            visible: admin && root.ip
            type: Kirigami.Heading.Type.Secondary
            verticalAlignment: Text.AlignTop
            elide: Text.ElideRight
        }
    }
}
