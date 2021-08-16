// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kmasto 1.0

Kirigami.ScrollablePage {
    id: timelinePage
    title: model.displayName

    required property var model

    property alias listViewHeader: listview.header
    property bool isProfile: false

    actions.main: Kirigami.Action {
        icon.name: "list-add"
        text: i18n("Toot")
        enabled: AccountManager.hasAccounts
        onTriggered: {
            const post = AccountManager.selectedAccount.newPost()
            pageStack.layers.push("qrc:/content/ui/TootComposer.qml", {
                postObject: post
            });
        }
    }

    ListView {
        id: listview
        model: timelinePage.model

        Component {
            id: fullScreenImage
            FullScreenImage {}
        }
        delegate: Kirigami.BasicListItem {
            topPadding: Kirigami.Units.largeSpacing
            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing
            highlighted: false
            hoverEnabled: false
            onClicked: pageStack.push("qrc:/content/ui/TimelinePage.qml", {
                model: model.threadModel
            })
            ListView.onReused: tootContent.visible = Qt.binding(() => { return model.spoilerText.length === 0; })
            contentItem: GridLayout {
                columnSpacing: Kirigami.Units.largeSpacing
                rowSpacing: 0
                columns: 2

                Kirigami.Icon {
                    source: "pin"
                    Layout.alignment: Qt.AlignRight
                    visible: model.pinned && timelinePage.isProfile
                    color: Kirigami.Theme.disabledTextColor
                    Layout.preferredHeight: Kirigami.Units.largeSpacing * 2
                    Layout.preferredWidth: Kirigami.Units.largeSpacing * 2
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                }
                QQC2.Label {
                    text: i18n("Pinned entry")
                    visible: model.pinned && timelinePage.isProfile
                    color: Kirigami.Theme.disabledTextColor
                    font: Kirigami.Theme.smallFont
                }

                Kirigami.Icon {
                    source: "retweet"
                    Layout.alignment: Qt.AlignRight
                    visible: model.wasReblogged
                    color: Kirigami.Theme.disabledTextColor
                    Layout.preferredHeight: Kirigami.Units.largeSpacing * 2
                    Layout.preferredWidth: Kirigami.Units.largeSpacing * 2
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                }
                QQC2.Label {
                    visible: model.wasReblogged
                    text: i18n("Shared by %1", model.rebloggedDisplayName)
                    color: Kirigami.Theme.disabledTextColor
                    font: Kirigami.Theme.smallFont
                }

                Kirigami.Avatar {
                    Layout.alignment: Qt.AlignTop
                    Layout.rowSpan: 4
                    source: model.avatar
                    cache: true
                    TapHandler {
                        onTapped: {
                            pageStack.push("qrc:/content/ui/AccountInfo.qml", {
                                model: model.accountModel
                            });
                        }
                    }
                    name: model.authorDisplayName
                }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    QQC2.Label {
                        text: model.authorDisplayName
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        color: Kirigami.Theme.disabledTextColor
                        text: `@${model.authorId}`
                    }
                    QQC2.Label {
                        text: model.relativeTime
                    }
                }
                ColumnLayout {
                    RowLayout {
                        visible: model.spoilerText.length !== 0
                        QQC2.Label {
                            Layout.fillWidth: true
                            text: model.spoilerText
                            wrapMode: Text.Wrap
                        }
                        QQC2.Button {
                            text: i18n("Show more")
                            onClicked: tootContent.visible = !tootContent.visible
                        }
                    }
                    TextEdit {
                        id: tootContent
                        Layout.fillWidth: true
                        text: model.display
                        textFormat: TextEdit.RichText
                        wrapMode: Text.Wrap
                        visible: model.spoilerText.length === 0
                        readOnly: true
                        selectByMouse: !Kirigami.Settings.isMobile
                        onLinkActivated: Qt.openUrlExternally(link)
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton // don't eat clicks on the Text
                            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                        }
                    }
                }

                GridLayout {
                    id: attachmentGrid
                    Layout.fillWidth: true
                    columns: model.attachments.length > 1 ? 2 : 1
                    Repeater {
                        model: attachments
                        Image {
                            Layout.fillWidth: true
                            Layout.maximumWidth: sourceSize.width
                            Layout.maximumHeight: width / sourceSize.width * sourceSize.height
                            source: modelData.previewUrl
                            cache: true
                            TapHandler {
                                onTapped: {
                                    fullScreenImage.createObject(parent, {
                                        model: attachments,
                                        currentIndex: index
                                    }).showFullScreen()
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    QQC2.ToolButton {
                        icon.name: "mail-replied-symbolic"
                        icon.width: Kirigami.Units.iconSizes.medium
                        icon.height: Kirigami.Units.iconSizes.medium
                        text: model.repliesCount < 2 ? model.repliesCount : "1+"
                        onClicked: {
                            const post = AccountManager.selectedAccount.newPost()
                            post.inReplyTo = model.id;
                            post.mentions = model.mentions;
                            if (!post.mentions.includes(`@${model.authorId}`)) {
                                post.mentions.push(`@${model.authorId}`);
                            }
                            pageStack.layers.push("qrc:/content/ui/TootComposer.qml", {
                                postObject: post
                            });
                        }
                        QQC2.ToolTip {
                            text: i18nc("Reply to a post", "Reply")
                        }
                    }
                    QQC2.ToolButton {
                        icon.source: model.reblogged ? 'qrc:/content/icon/boost-post-done.svg' : 'qrc:/content/icon/boost-post.svg'
                        icon.width: Kirigami.Units.iconSizes.medium
                        icon.height: Kirigami.Units.iconSizes.medium
                        onClicked: timelineModel.actionRepeat(timelineModel.index(model.index, 0))
                        QQC2.ToolTip {
                            text: i18nc("Share a post", "Boost")
                        }
                    }
                    QQC2.ToolButton {
                        icon.source: model.favorite ? 'qrc:/content/icon/like-post-done.svg' : 'qrc:/content/icon/like-post.svg'
                        icon.width: Kirigami.Units.iconSizes.medium
                        icon.height: Kirigami.Units.iconSizes.medium
                        onClicked: timelineModel.actionFavorite(timelineModel.index(model.index, 0))
                        QQC2.ToolTip {
                            text: i18nc("Like a post", "Like")
                        }
                    }
                }
            }
        }
    }
}
