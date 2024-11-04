import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Client 1.0 as Client

Window {
    width: 280
    height: 520
    visible: true
    title: qsTr("voice app")

    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: textfield.top
                margins: 20
            }

            Label {
                text: "your Id: " + "client1"
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }

            Label {
                text: "other Id: " + textfield.text
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }
        }

        TextField {
            id: textfield
            placeholderText: "Enter the Id"
            anchors.bottom: callbtn.top
            anchors.bottomMargin: 10
            anchors.left: callbtn.left
            anchors.right: callbtn.right
            enabled: !callbtn.pushed
        }

        Button {
            id: callbtn

            property bool pushed: false

            text: "Call"
            Material.background: "green"
            Material.foreground: "white"
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: 20
            }

            onClicked: {
                pushed = !pushed
                if (pushed) {
                    Client.Client.offerCall(textfield.text)
                    Material.background = "red"
                    text = "End Call"
                }
                else {
                    Client.Client.endCall()
                    Material.background = "green"
                    text = "Call"
                    textfield.clear()
                }
            }
        }
    }

    Connections {
            target: Client.Client

            onCallReceived: {
                callbtn.pushed = true
                callbtn.Material.background = "red"
                callbtn.text = "End Call"
            }
        }
    Connections {
            target: Client.Client

            onCallEnded: {
                callbtn.pushed = false
                callbtn.Material.background = "green"
                callbtn.text = "Call"
                callbtn.textfield.clear()
            }
        }
}
