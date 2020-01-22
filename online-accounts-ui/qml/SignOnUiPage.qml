import QtQuick 2.9
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3 as ListItem
import Ubuntu.OnlineAccounts.Plugin 1.0

MainView {
    id: root

    property var signonRequest: request

    width: units.gu(60)
    height: units.gu(90)

    Page {
        title: signonRequest.title

        WebView {
            id: loader
            signonRequest: root.signonRequest

            anchors {
                fill: parent
                bottomMargin: Math.max(osk.height, cancelButton.height)
            }
        }

        ListItem.SingleControl {
            id: cancelButton
            anchors.bottom: parent.bottom
            showDivider: false
            control: Button {
                text: i18n.dtr("ubuntu-system-settings-online-accounts", "Cancel")
                width: parent.width - units.gu(4)
                onClicked: signonRequest.cancel()
            }
        }

        KeyboardRectangle {
            id: osk
        }
    }
}
