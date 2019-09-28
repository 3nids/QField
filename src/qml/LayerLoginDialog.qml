import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.3

import org.qfield 1.0
import "js/style.js" as Style
import "."// as QField

Item {
  signal enter( string usr, string pw )
  signal cancel()

  property var realm
  property var inCancelation

  ToolBar {
    id: toolbar
    height: 48 * dp
    visible: true

    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }

    background: Rectangle {
      color: '#80CC28'
    }

    RowLayout {
      anchors.fill: parent
      Layout.margins: 0

      Button {
        id: enterButton

        Layout.alignment: Qt.AlignTop | Qt.AlignLeft

        visible: form.state === 'Add' || form.state === 'Edit'
        width: 48*dp
        height: 48*dp
        clip: true
        bgcolor: "#212121"

        iconSource: Style.getThemeIcon( 'ic_check_white_48dp' )

        onClicked: {
          enter(username.text, password.text)
          username.text=''
          password.text=''
        }
      }

      Label {
        id: titleLabel

        text: "Login information"
        font.pointSize: 14
        font.bold: true
        color: "#000000"
        elide: Label.ElideRight
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        Layout.fillWidth: true
      }

      Button {
        id: cancelButton

        Layout.alignment: Qt.AlignTop | Qt.AlignRight

        width: 49*dp
        height: 48*dp
        clip: true
        bgcolor: form.state === 'Add' ? "#900000" : "#212121"

        iconSource: Style.getThemeIcon( 'ic_close_white_24dp' )

        onClicked: {
          cancel()
        }
      }
    }
  }

    ColumnLayout{
      anchors.fill: parent
      Layout.fillWidth: true
      Layout.fillHeight: true

      spacing: 2
      anchors {
          margins: 4 * dp
          topMargin: 52 * dp // Leave space for the toolbar
      }

      Text {
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.preferredHeight: font.height + 20 * dp
        text: realm
        font.pointSize: 14
        font.bold: true
      }

      Text {
        id: usernamelabel
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.preferredHeight: font.height
        text: qsTr( "Username" )
        font.pointSize: 14
      }

      TextField {
        id: username
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.preferredWidth: Math.max( parent.width / 2, usernamelabel.width )
        Layout.preferredHeight: font.height + 20 * dp
        font.pointSize: 14

        background: Rectangle {
          y: username.height - height - username.bottomPadding / 2
          implicitWidth: 120 * dp
          height: username.activeFocus ? 2 * dp : 1 * dp
          color: username.activeFocus ? "#4CAF50" : "#C8E6C9"
        }
      }

      Item {
          // spacer item
          height: 35 * dp
      }

      Text {
        id: passwordlabel
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.preferredHeight: font.height
        text: qsTr( "Password" )
        font.pointSize: 14
      }

      TextField {
        id: password
        echoMode: TextInput.Password
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.preferredWidth: Math.max( parent.width / 2, usernamelabel.width )
        Layout.preferredHeight: font.height + 20 * dp
        height: font.height + 20 * dp
        font.pointSize: 14

        background: Rectangle {
          y: password.height - height - password.bottomPadding / 2
          implicitWidth: 120 * dp
          height: password.activeFocus ? 2 * dp : 1 * dp
          color: password.activeFocus ? "#4CAF50" : "#C8E6C9"
        }
      }

      Item {
          // spacer item
          Layout.fillWidth: true
          Layout.fillHeight: true
      }
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
