import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Window {
    width: 280
    height: 520
    visible: true
    title: qsTr("low_mist")

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
                text: "Ip: " + "172.16.142.176"
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }

            Label {
                text: "IceCandidate: " + "172.16.142.176"
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }

            Label {
                text: "CallerId: " + textfield.text
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }

        }

        TextField {
            id: textfield
            placeholderText: "Phone Number"
            anchors.bottom: callbtn.top
            anchors.bottomMargin: 10
            anchors.left: callbtn.left
            anchors.right: callbtn.right
            enabled: !callbtn.pushed
        }

        Button {
            id: callbtn

            property bool pushed: false

            height: 47
            text: "Call"
            Material.background: "green"
            Material.foreground: "white"
            anchors{
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: 20
            }

            onClicked: {
                pushed = !pushed
                if(pushed){
                    Material.background = "red"
                    text = "End Call"
                }
                else{
                    Material.background = "green"
                    text = "Call"
                    textfield.clear()
                }
            }
        }
    }
}
