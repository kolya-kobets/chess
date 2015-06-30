import QtQuick 2.0

Rectangle {
    property alias cln_name : cln_text.text
    Text {
        id :cln_text
        text: cln_name
        anchors.centerIn : parent
        font.bold: true
        font.pointSize: 14
    }
}

