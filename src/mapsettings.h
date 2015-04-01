/***************************************************************************

               ----------------------------------------------------
              date                 : 27.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias.kuhn (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAPSETTINGS_H
#define MAPSETTINGS_H

#include <QObject>

#include <qgsrectangle.h>
#include <qgsmapcanvas.h>

#include "crs.h"

class MapSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QgsRectangle extent READ extent WRITE setExtent NOTIFY extentChanged )
    Q_PROPERTY( CRS* crs READ crs NOTIFY crsChanged )
    // Q_PROPERTY( QPointF center READ center WRITE setCenter NOTIFY centerChanged )
    // Q_PROPERTY( qreal scale READ scale WRITE setScale NOTIFY scaleChanged )

  public:
    MapSettings( QObject* parent = 0 );
    ~MapSettings();

    const QgsRectangle extent() const;
    void setExtent( const QgsRectangle& extent );

    // TODO: We don't want to really store this in here
    void setQgsMapCanvas( QgsMapCanvas* mapCanvas );
    QgsMapCanvas* qgsMapCanvas();

    Q_INVOKABLE void setCenter( const QPointF& center );

    double mapUnitsPerPixel();
    const QgsRectangle visibleExtent();

    CRS* crs() const;

    /**
     * Convert a map coordinate to screen pixel coordinates
     *
     * @param p A coordinate in map coordinates
     *
     * @return A coordinate in pixel / screen space
     */
    Q_INVOKABLE const QPointF coordinateToScreen( const QPointF& p ) const;


    /**
     * Convert a screen coordinate to a map coordinate
     *
     * @param p A coordinate in pixel / screen coordinates
     *
     * @return A coordinate in map coordinates
     */
    Q_INVOKABLE const QPointF screenToCoordinate( const QPointF& p ) const;

  signals:
    void extentChanged();
    void crsChanged();
    // void centerChanged();
    // void scaleChanged();

  private slots:
    void onMapCrsChanged();

  private:
    // As of now, this will be used to own and handle the mapSettings
    // THIS SHOULD BE CHANGED!
    QgsMapCanvas* mMapCanvas;
    CRS* mCrs;
};

#endif // MAPSETTINGS_H
