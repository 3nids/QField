import QtQuick 2.11
import org.qgis 1.0
import org.qfield 1.0


Item {
  id: geometryRenderer
  property alias geometry: geometry
  property double lineWidth: 8 * dp
  property color color: "yellow"
  property double pointSize: 20 * dp
  property color borderColor: "blue"
  property double borderSize: 2 * dp
  property MapSettings mapSettings: mapCanvas.mapSettings

  QgsGeometryWrapper {
    id: geometry
  }

  Connections {
    target: geometry

    onGeometryChanged: {
      geometryComponent.sourceComponent = undefined
      if (geometry && geometry.qgsGeometry.type === QgsWkbTypes.PointGeometry) {
        geometryComponent.sourceComponent = pointHighlight
      }
      else
      {
        geometryComponent.sourceComponent = linePolygonHighlight
      }
    }
  }

  Component {
    id: linePolygonHighlight

    LinePolygonHighlight {
      id: linePolygonHighlightItem
      mapSettings: geometryRenderer.mapSettings

      transform: MapTransform {
        mapSettings: geometryRenderer.mapSettings
      }

      geometry: geometryRenderer.geometry
      color: geometryRenderer.color
      width: geometryRenderer.lineWidth
    }
  }

  Component {
    id: pointHighlight

    Repeater {
      model: geometry.pointList()

      Rectangle {
        property CoordinateTransformer ct: CoordinateTransformer {
          id: _ct
          sourceCrs: geometry.crs
          sourcePosition: modelData
          destinationCrs: mapCanvas.mapSettings.destinationCrs
          transformContext: qgisProject.transformContext
        }

        MapToScreen {
          id: mapToScreen
          mapSettings: mapCanvas.mapSettings
          mapPoint: _ct.projectedPosition
        }

        x: mapToScreen.screenPoint.x - width/2
        y: mapToScreen.screenPoint.y - width/2

        color: geometryRenderer.color
        width: geometryRenderer.pointSize
        height: geometryRenderer.pointSize
        radius: geometryRenderer.pointSize / 2

        border.color: geometryRenderer.borderColor
        border.width: geometryRenderer.borderSize
      }
    }
  }

  Loader {
    id: geometryComponent
    sourceComponent: geometry && geometry.qgsGeometry.type === QgsWkbTypes.PointGeometry ? pointHighlight : linePolygonHighlight
  }

}
