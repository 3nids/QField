import QtQuick 2.12
import Qt.labs.settings 1.0 as LabSettings

LabSettings.Settings {
  property bool positioningActivated: false
  property string positioningDeviceName: qsTr( "Internal device" )
  property real antennaHeight: 0.0
  property bool antennaHeightActivated: false
  property bool skipAltitudeCorrection: false
  property bool showPositionInformation: false
}
