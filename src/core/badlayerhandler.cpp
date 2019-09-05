/***************************************************************************
  badlayerhandler.cpp - BadLayerHandler

 ---------------------
 begin                : 3.10.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "badlayerhandler.h"
#include <qgsproject.h>
#include <QDebug>
#include "qgscredentials.h"

BadLayerHandler::BadLayerHandler( QObject *parent )
  : QStandardItemModel( parent )
{

}

QHash<int, QByteArray> BadLayerHandler::roleNames() const
{
  QHash<int, QByteArray> roleNames = QStandardItemModel::roleNames();

  roleNames[DataSourceRole] = "DataSource";
  roleNames[LayerNameRole] = "LayerName";
  roleNames[LayerNodeRole] = "LayerNode";

  return roleNames;
}

QgsProject *BadLayerHandler::project() const
{
  return mProject;
}

void BadLayerHandler::setProject( QgsProject *project )
{
  if ( mProject == project )
    return;

  mProject = project;

  mProject->setBadLayerHandler( this );
  emit projectChanged();
}

void BadLayerHandler::handleBadLayers( const QList<QDomNode> &layers )
{
  clear();

  for ( const QDomNode &node : layers )
  {
    QStandardItem *item = new QStandardItem();
    item->setData( dataSource( node ), DataSourceRole );
    item->setData( layerName( node ), LayerNameRole );
    //item->setData( node, LayerNodeRole );
    appendRow( item );
  }

  //emit badLayersFound();

  //QgsCredentials::instance()->put( QStringLiteral("allow-groups at wms.swisstopo.admin.ch"), QStringLiteral("user_bsv0a"), QStringLiteral("2581qmtmq0t2schz") );

  for ( const QDomNode &node : layers )
  {
    QDomNode layerNode = node;
    //setDataSource( layerNode, dataSource(node) );
    const QString layerId { layerNode.namedItem( QStringLiteral( "id" ) ).toElement().text() };
    const QString provider { layerNode.namedItem( QStringLiteral( "provider" ) ).toElement().text() };
    QgsMapLayer *mapLayer = QgsProject::instance()->mapLayer( layerId );
    QgsDataProvider::ProviderOptions options;
    const auto absolutePath { QgsProject::instance()->pathResolver().readPath( "contextualWMSLegend=0&crs=EPSG:31287&dpiMode=7&featureCount=10&format=image/jpeg&layers=ch.swisstopo.swissbathy3d-reliefschattierung&password=2581qmtmq0t2schz&styles=default&url=https://wms.swisstopo.admin.ch/?&username=user_bsv0a" ) };
    mapLayer->setDataSource( absolutePath, layerName( layerNode ), provider, options );

    mapLayer->reload();
    if ( mapLayer->isValid() )
    {
      qDebug() << "trallala";
    }
  }
}

QString BadLayerHandler::layerName( const QDomNode &layerNode ) const
{
  return layerNode.namedItem( "layername" ).toElement().text();
}

//void BadLayerHandler::setDataSource(QDomNode &layerNode, const QString &dataSource)
//{
//  QgsProjectBadLayerHandler::setDataSource(layerNode, "contextualWMSLegend=0&crs=EPSG:31287&dpiMode=7&featureCount=10&format=image/jpeg&layers=ch.swisstopo.swissbathy3d-reliefschattierung&password=2581qmtmq0t2schz&styles=default&url=https://wms.swisstopo.admin.ch/?&username=user_bsv0a");
//}
