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
import SystemSettings 1.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.OnlineAccounts 0.1

ItemPage {
    id: root

    property string domain: "ubuntu-system-settings-online-accounts"

    objectName: "accountsPage"

    title: i18n.dtr(domain, "Accounts")

    property Item flickable: accountsPage.visible ? accountsPage : noAccountsPage

    AccountServiceModel {
        id: accountsModel
        service: "global"
        includeDisabled: true
    }
    
    AccountsPage {
        id: accountsPage
        anchors.fill: parent
        accountsModel: accountsModel
        visible: accountsModel.count > 0
    }
    
    NoAccountsPage {
        id: noAccountsPage
        anchors.fill: parent
        accountsModel: accountsModel
        visible:!accountsPage.visible
    }
}