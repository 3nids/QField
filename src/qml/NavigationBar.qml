import QtQuick 2.0
import QtQuick.Controls 1.2
import org.qgis 1.0

Rectangle {
  id: toolBar

  property string currentName: ''
  property bool showNavigationButtons
  property FeatureListModel model
  property FeatureListModelSelection selection

  signal gotoNext
  signal gotoPrevious
  signal statusIndicatorClicked

  anchors.top:parent.top
  anchors.left: parent.left
  anchors.right: parent.right
  height: 48*dp

  clip: true

  Rectangle {
    id: navigationStatusIndicator
    anchors.fill: parent
    height: 48*dp

    color: "#80CC28"

    clip: true

    focus: true

    Text {
      anchors.centerIn: parent

      text: ( selection.selection + 1 ) + '/' + model.count + ': ' + currentName
    }

    MouseArea {
      anchors.fill: parent

      onClicked: {
        toolBar.statusIndicatorClicked()
      }
    }
  }

  Button {
    id: nextButton

    anchors.right: parent.right

    width: ( parent.showNavigationButtons ? 48*dp : 0 )
    height: 48*dp

    iconSource: "/themes/holodark/next_item.png"

    enabled: ( ( selection.selection + 1 ) < toolBar.model.count )

    onClicked: {
      selection.selection = selection.selection + 1
    }

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }

  Button {
    id: previousButton

    anchors.left: parent.left

    width: ( parent.showNavigationButtons ? 48*dp : 0 )
    height: 48*dp

    iconSource: "/themes/holodark/previous_item.png"

    enabled: ( selection.selection > 0 )

    onClicked: {
      selection.selection = selection.selection - 1
    }

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }
}
