/***************************************************************************
                            FeatureForm.qml
                              -------------------
              begin                : 10.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias.kuhn (at) opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import org.qgis 1.0

Rectangle {
  id: featureForm

  property FeatureListModelSelection selection

  states: [
    State {
      name: "Hidden"
      StateChangeScript {
        script: hide()
      }
    },
    State {
      name: "FeatureList"
      PropertyChanges {
        target: globalFeaturesList
        shown: true
      }
      PropertyChanges {
        target: featureListToolBar
        showNavigationButtons: false
      }
      StateChangeScript {
        script: show()
      }
    },
    State {
      name: "FeatureForm"
      PropertyChanges {
        target: globalFeaturesList
        shown: false
      }
      PropertyChanges {
        target: featureListToolBar
        showNavigationButtons: true
      }
    }
  ]
  state: "Hidden"

  focus: ( state != "Hidden" )

  clip: true

  ListView {
    id: globalFeaturesList

    anchors.top: featureListToolBar.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    height: parent.height - featureListToolBar.height

    property bool shown: false

    clip: true

    model: featureListModel
    section.property: "layerName"
    section.delegate: Component {
      Rectangle {
        width: parent.width
        height: 30*dp

        Text {
          anchors { left: parent.left; leftMargin: 10 }
          font.bold: true
          text: section
        }
      }
    }

    delegate: Item {
      anchors { left: parent.left; right: parent.right }

      focus: true

      height: 48*dp
      Text {
        anchors { leftMargin: 10; left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        text: "<b>" + display + "</b>"
      }

      Rectangle {
        anchors.left: parent.left
        height: parent.height
        width: 6
        color: "darkGray"
        opacity: ( index == featureForm.selection.selection )
        Behavior on opacity {
          PropertyAnimation {
            easing.type: Easing.InQuart
          }
        }
      }

      /* bottom border */
      Rectangle {
        anchors.bottom: parent.bottom
        height: 1
        color: "lightGray"
        width: parent.width
      }

      MouseArea {
        anchors.fill: parent

        onClicked: {
          featureForm.selection.selection = index
          featureForm.state = "FeatureForm"
        }

        onPressAndHold:
        {
          featureForm.selection.selection = index
        }
      }
    }

    /* bottom border */
    Rectangle {
      anchors.bottom: parent.bottom
      height: 1
      color: "lightGray"
      width: parent.width
    }

    onShownChanged: {
      if ( shown )
      {
        height = parent.height - featureListToolBar.height
      }
      else
      {
        height = 0
      }
    }

    Behavior on height {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }

  ListView {
    id: featureFormList

    anchors.top: featureListToolBar.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    model: FeatureModel {
      feature: featureForm.selection.selectedFeature
    }

    focus: true

    visible: (!globalFeaturesList.shown)

    delegate: Item {
      height: 30
      anchors.left: parent.left
      anchors.right: parent.right

      Rectangle {
        anchors.fill: parent

        Row {
          Text {
            anchors.leftMargin: 5
            width: featureFormList.width / 3
            text: "<b>" + attributeName + "</b>"
            clip: true
          }

          TextEdit {
            anchors.rightMargin: 5
            text: attributeValue
            /* onTextChanged: { feature[index].value = text; } */
          }
        }

        /* Bottom border */
        Rectangle {
          height: 1
          color: "lightGray"
          width: parent.width
          anchors.bottom: parent.bottom
        }
      }
    }
  }

  NavigationBar {
    id: featureListToolBar
    model: featureListModel
    selection: featureForm.selection
  }

  Keys.onReleased: {
    if (event.key === Qt.Key_Back) {
      featureForm.hide()
      event.accepted = true
    }
  }

  Behavior on width {
    PropertyAnimation {
      easing.type: Easing.InQuart
    }
  }

  Connections {
    target: featureListModel

    onModelReset: {
      state = "FeatureList"
    }
  }

  Connections {
    target: featureModel

    onModelReset: {
      state = "FeatureForm"
    }
  }

  function show()
  {
    var widthDenominator = settings.value( "featureForm/widthDenominator", 3 );
    width = parent.width / widthDenominator
    // Focus to retrieve back button events
    focus = true
  }

  function hide()
  {
    width = 0
    focus = false
  }
}
