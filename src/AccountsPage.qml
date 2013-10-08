/*
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Flickable {
    id: root
    property variant accountsModel
    contentHeight: contentItem.childrenRect.height
    boundsBehavior: Flickable.StopAtBounds

    Column {
        anchors.left: parent.left
        anchors.right: parent.right

        ListView {
            anchors.left: parent.left
            anchors.right: parent.right
            interactive: false
            height: contentHeight
            model: accountsModel

            delegate: AccountItem {
                ListView.delayRemove: running
                text: providerName
                subText: displayName
                accountHandle: account
                globalServiceHandle: accountService
            }
        }

        ListItem.SingleControl {
            control: Button {
                text: i18n.dtr(plugin.translations, "Add account…")
                width: parent.width - units.gu(4)
                onClicked: pageStack.push(newAccountPage)
            }
            showDivider: false
        }

        AddAccountLabel {}
    }

    Component {
        id: newAccountPage
        NewAccountPage {}
    }
}
