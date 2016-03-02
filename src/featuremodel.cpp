/***************************************************************************
                            featuremodel.cpp
                              -------------------
              begin                : 10.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "featuremodel.h"

#include "feature.h"

FeatureModel::FeatureModel( QObject *parent ) :
  QAbstractListModel( parent )
{
  connect( this, SIGNAL( modelReset() ), this, SIGNAL( featureChanged() ) );
}

void FeatureModel::setFeature( const QVariant& v )
{
  const Feature& feature = v.value<Feature>();
  setFeature( feature );
}

void FeatureModel::setFeature( const Feature& feature, bool force )
{
  if ( feature.layer() != mFeature.layer() || feature.id() != mFeature.id() || force )
  {
    beginResetModel();
    mFeature = feature;
    endResetModel();
  }
}

void FeatureModel::setLayer( QgsVectorLayer* layer )
{
  if ( layer != mFeature.layer() )
    mFeature.setLayer( layer );

  setFeature( mFeature, true );
}

QgsVectorLayer* FeatureModel::layer() const
{
  return mFeature.layer();
}

Feature FeatureModel::feature() const
{
  return mFeature;
}

QHash<int, QByteArray> FeatureModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
  roles[AttributeName]  = "attributeName";
  roles[AttributeValue] = "attributeValue";
  roles[EditorWidget] = "editorWidget";
  roles[EditorWidgetConfig] = "editorWidgetConfig";

  return roles;
}


int FeatureModel::rowCount( const QModelIndex& parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return mFeature.attributes().count();
}

QVariant FeatureModel::data( const QModelIndex& index, int role ) const
{
  switch ( role )
  {
    case AttributeName:
      return mFeature.fields()->at( index.row() ).name();
      break;

    case AttributeValue:
      return mFeature.attribute( index.row() );
      break;

    case AttributeEditable:
      return false;
      break;

    case EditorWidget:
      return mFeature.layer()->editFormConfig()->widgetType( index.row() );
      break;

    case EditorWidgetConfig:
      return mFeature.layer()->editFormConfig()->widgetConfig( index.row() );
  }

  return QVariant();
}


void FeatureModel::setAttribute( int fieldIndex, const QVariant& value )
{
  mFeature.setAttribute( fieldIndex, value );
}

bool FeatureModel::save()
{
  mFeature.layer()->startEditing();
  QgsFeature feat = mFeature.qgsFeature();
  mFeature.layer()->updateFeature( feat );
  bool rv = mFeature.layer()->commitChanges();
  if ( rv )
  {
    QgsFeature feat;
    if ( mFeature.layer()->getFeatures( QgsFeatureRequest().setFilterFid( mFeature.id() ) ).nextFeature( feat ) )
      setFeature( Feature( feat, mFeature.layer() ), true );
  }
  return rv;
}

void FeatureModel::reset()
{
  mFeature.layer()->rollBack();
}

bool FeatureModel::suppressFeatureForm() const
{
  return mFeature.layer()->editFormConfig()->suppress();
}

void FeatureModel::applyGeometry()
{
  mFeature.setGeometry( mGeometry->asQgsGeometry() );
}

void FeatureModel::create()
{
  mFeature.layer()->startEditing(); // better safe than sorry
  mFeature.create();
}
