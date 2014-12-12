/***************************************************************************
                            NavigationBar.qml
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


import QtQuick 2.0
import QtQuick.Controls 1.2
import org.qgis 1.0

Rectangle {
  id: toolBar

  property string currentName: ''
  property bool showEditButtons
  property FeatureListModel model
  property FeatureListModelSelection selection

  signal statusIndicatorClicked
  signal editButtonClicked
  signal save
  signal cancel

  anchors.top:parent.top
  anchors.left: parent.left
  anchors.right: parent.right
  height: 48*dp

  clip: true

  states: [
    State {
      name: "Navigation"
    },
    State {
      name: "Indication"
    },
    State {
      name: "Edit"
    }
  ]

  state: "Indication"

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

    width: ( parent.state == "Navigation" ? 48*dp : 0 )
    height: 48*dp
    clip: true

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
    id: saveButton

    anchors.right: parent.right

    width: ( parent.state == "Edit" ? 48*dp : 0 )
    height: 48*dp
    clip: true

    iconSource: "/themes/holodark/accept.png"

    onClicked: {
      toolBar.save()
    }

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }

  Button {
    id: cancelButton

    anchors.left: parent.left

    width: ( parent.state == "Edit" ? 48*dp : 0 )
    height: 48*dp
    clip: true

    iconSource: "/themes/holodark/cancel.png"

    onClicked: {
      toolBar.cancel()
    }

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }

  Button {
    id: editButton

    anchors.right: nextButton.left

    width: ( parent.state == "Navigation" ? 48*dp : 0 )
    height: 48*dp
    clip: true

    iconSource: "/themes/holodark/edit.png"

    onClicked: {
      toolBar.editButtonClicked()
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

    width: ( parent.state == "Navigation" ? 48*dp : 0 )
    height: 48*dp
    clip: true

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
