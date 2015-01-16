#ifndef QGSQMLINTERFACE_H
#define QGSQMLINTERFACE_H

#include <QObject>
#include <QPointF>

#include <QStandardItemModel>

class QgisMobileapp;

class QgsQmlInterface : public QObject
{
    Q_OBJECT

  public:
    QgsQmlInterface( QgisMobileapp* app );
    QgsQmlInterface()
    {
      // You shouldn't get here, this constructor only exists that we can register it as a QML type
      Q_ASSERT( false );
    }

  public Q_SLOTS:
    /**
     * When called, it will request any features at this position and after doing so emit a
     * featuresIdentified signal to which you may connect.
     *
     * @param point Coordinate at which to identify features
     */
    void identifyFeatures( const QPointF point );

    void openProjectDialog();

  private:
    QgisMobileapp* mApp;
};

#endif // QGSQMLINTERFACE_H
