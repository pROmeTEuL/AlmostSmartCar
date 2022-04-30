import QtQuick 2.12
import QtQuick.Window 2.12

Rectangle {
    property double xAxis: thumb.anchors.horizontalCenterOffset / radius
    property double yAxis: thumb.anchors.verticalCenterOffset / radius

    anchors.margins: 10 * Screen.pixelDensity
    width: 30 * Screen.pixelDensity // 1.5 , 3cm
    height: width
    
    color: Qt.rgba(0.5, 0.5, 0.5, 0.75)
    radius: width / 2
    ParallelAnimation {
        id: returnAnimation
        NumberAnimation { target: thumb.anchors; property: "horizontalCenterOffset";
            to: 0; duration: 100; easing.type: Easing.OutSine }
        NumberAnimation { target: thumb.anchors; property: "verticalCenterOffset";
            to: 0; duration: 100; easing.type: Easing.OutSine }
    }
    
    MultiPointTouchArea {
        id: touch
        anchors.fill: parent
        touchPoints: TouchPoint {
            property real fingerAngle : Math.atan2(x, y)
            property int mcx : x - touch.width * 0.5
            property int mcy : y - touch.height * 0.5
            property bool fingerOutOfBounds : fingerDistance2 < distanceBound2
            property real fingerDistance2 : mcx * mcx + mcy * mcy
            property real distanceBound : touch.width * 0.5
            property real distanceBound2 : distanceBound * distanceBound
            onPressedChanged: pressed ? returnAnimation.stop() : returnAnimation.restart()
            onAreaChanged: {
                if (fingerOutOfBounds) {
                    thumb.anchors.horizontalCenterOffset = mcx
                    thumb.anchors.verticalCenterOffset = mcy
                } else {
                    var angle = Math.atan2(mcy, mcx)
                    thumb.anchors.horizontalCenterOffset = Math.cos(angle) * distanceBound
                    thumb.anchors.verticalCenterOffset = Math.sin(angle) * distanceBound
                }
            }
        }
    }
    
    Rectangle {
        id: thumb
        color: Qt.rgba(0, 0.5, 0.5, 0.75)
        width: 15 * Screen.pixelDensity // 0.7, 1.5 cm
        height: width
        radius: width / 2
        anchors.centerIn: parent
    }
}
