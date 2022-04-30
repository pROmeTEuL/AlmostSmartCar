import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Car controller")
    Button {
        anchors.left: parent.left
        anchors.top: parent.top
        visible: !_control.connected
        text: "Connect"
        onClicked: _control.connectToCar()
    }
    Button {
        visible: _control.connected
        anchors.right: parent.right
        anchors.top: parent.top
        text: "SHUT DOWN"
        onClicked: _control.shutdownServer();
    }
    Button {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: "Quit"
        onClicked: Qt.quit();
    }
    ColumnLayout {
        visible: _control.connected
        anchors.centerIn: parent
        Label {
            Layout.alignment: Qt.AlignHCenter
            text: "Front: " + _control.frontS + "cm"
        }
        RowLayout {
            spacing: 5
            Layout.alignment: Qt.AlignHCenter
            Label {
                text: "Left: " + _control.leftS + "cm"
            }

            Label{
                text: "Right: " + _control.rightS + "cm"
            }
        }
        Label{
            Layout.alignment: Qt.AlignHCenter
            text: "Rear: " + _control.rearS + "cm"
        }
    }
    OnScreenJoystick {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        visible: _control.connected
        onXAxisChanged: _control.setJoyPos(xAxis, yAxis)
        onYAxisChanged: _control.setJoyPos(xAxis, yAxis)
    }
}
