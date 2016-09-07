/***************************************************************************
  attributeformmodel.cpp - AttributeFormModel

 ---------------------
 begin                : 16.8.2016
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

#include "attributeformmodel.h"
#include "qgsvectorlayer.h"

#include <QDebug>

AttributeFormModel::AttributeFormModel( QObject* parent )
  : QAbstractItemModel( parent )
  , mFeatureModel( nullptr )
  , mLayer( nullptr )
  , mTemporaryContainer( nullptr )
{
}

AttributeFormModel::~AttributeFormModel()
{
  delete mTemporaryContainer;
}

QModelIndex AttributeFormModel::index( int row, int column, const QModelIndex& parent ) const
{
  if ( row < 0 )
    return QModelIndex();
  QgsAttributeEditorContainer* container;
  if ( !parent.isValid() )
  {
    container = invisibleRootContainer();
  }
  else
  {
    container = indexToElement<QgsAttributeEditorContainer*>( parent );
  }
  return createIndex( row, column, container->children().at( row ) );
}

QModelIndex AttributeFormModel::parent( const QModelIndex& index ) const
{
  return createIndex( 0, 0, indexToElement( index )->parent() );
}

QHash<int, QByteArray> AttributeFormModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

  roles[ElementType]  = "Type";
  roles[Name]  = "Name";
  roles[AttributeValue] = "AttributeValue";
  roles[EditorWidget] = "EditorWidget";
  roles[EditorWidgetConfig] = "EditorWidgetConfig";
  roles[RememberValue] = "RememberValue";
  roles[Field] = "Field";

  return roles;
}

int AttributeFormModel::rowCount( const QModelIndex& parent ) const
{
  if ( !mLayer )
    return 0;

  QgsAttributeEditorElement* element;
  if ( !parent.isValid() )
    element = invisibleRootContainer();
  else
    element = indexToElement( parent );

  if ( element->type() == QgsAttributeEditorElement::AeTypeContainer )
  {
    QgsAttributeEditorContainer* container = static_cast<QgsAttributeEditorContainer*>( element );
    if ( container )
      return container->children().length();
  }

  return 0;
}

int AttributeFormModel::columnCount( const QModelIndex& parent ) const
{
  if ( !parent.isValid() )
    return 0;

  return 1;
}

QVariant AttributeFormModel::data( const QModelIndex& index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( role == ElementType )
  {
    switch ( indexToElement( index )->type() )
    {
      case QgsAttributeEditorElement::AeTypeContainer:
        return "container";
        break;

      case QgsAttributeEditorElement::AeTypeField:
        return "field";
        break;

      case QgsAttributeEditorElement::AeTypeRelation:
        return "relation";
        break;

      case QgsAttributeEditorElement::AeTypeInvalid:
        return QVariant();
        break;
    }
  }

  switch( indexToElement( index )->type() )
  {
    case QgsAttributeEditorElement::AeTypeContainer:
    {
      QgsAttributeEditorContainer* container = indexToElement<QgsAttributeEditorContainer*>( index );

      switch ( role )
      {
        case Name:
          return container->name();
      }
      break;
    }

    case QgsAttributeEditorElement::AeTypeField:
    {
      QgsAttributeEditorField* editorField = indexToElement<QgsAttributeEditorField*>( index );
      if ( editorField->idx() < 0 )
        return QVariant();

      switch ( role )
      {
        case Name:
          return mLayer->attributeDisplayName( editorField->idx() );

        case AttributeValue:
          return mFeatureModel->feature().attribute( editorField->idx() );

        case AttributeEditable:
          return mLayer->editFormConfig().readOnly( editorField->idx() );

        case EditorWidget:
          return mLayer->editFormConfig().widgetType( editorField->name() );

        case EditorWidgetConfig:
          return mLayer->editFormConfig().widgetConfig( editorField->name() );

        case RememberValue:
          return mFeatureModel->rememberedAttributes().at ( editorField->idx() ) ? Qt::Checked : Qt::Unchecked;

        case Field:
          return mLayer->fields().at( editorField->idx() );

        case Qt::DisplayRole:
          return mLayer->attributeDisplayName( editorField->idx() );
      }
      break;
    }

    case QgsAttributeEditorElement::AeTypeRelation:
      // TODO
      break;

    case QgsAttributeEditorElement::AeTypeInvalid:
      break;
  }

  return QVariant();
}

bool AttributeFormModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  if ( data( index, role ) != value )
  {
    switch ( role )
    {
      case RememberValue:
      {
        QgsAttributeEditorField* editorField = indexToElement<QgsAttributeEditorField*>( index );
        return mFeatureModel->setData( mFeatureModel->index( editorField->idx() ), value, FeatureModel::RememberAttribute );
        break;
      }

      case AttributeValue:
      {
        QgsAttributeEditorField* editorField = indexToElement<QgsAttributeEditorField*>( index );
        return mFeatureModel->setData( mFeatureModel->index( editorField->idx() ), value, FeatureModel::AttributeValue );
        break;
      }
    }
  }
  return false;
}

FeatureModel* AttributeFormModel::featureModel() const
{
  return mFeatureModel;
}

void AttributeFormModel::setFeatureModel( FeatureModel* featureModel )
{
  if ( mFeatureModel == featureModel )
    return;

  disconnect( mFeatureModel, &FeatureModel::layerChanged, this, &AttributeFormModel::onLayerChanged );
  disconnect( mFeatureModel, &FeatureModel::featureChanged, this, &AttributeFormModel::onFeatureChanged );


  mFeatureModel = featureModel;

  connect( mFeatureModel, &FeatureModel::layerChanged, this, &AttributeFormModel::onLayerChanged );
  connect( mFeatureModel, &FeatureModel::featureChanged, this, &AttributeFormModel::onFeatureChanged );

  emit featureModelChanged();
}

void AttributeFormModel::onLayerChanged()
{
  beginResetModel();
  mLayer = mFeatureModel->layer();

  if ( mLayer )
  {
    QgsAttributeEditorContainer* root;
    delete mTemporaryContainer;
    mTemporaryContainer = nullptr;

    if ( mLayer->editFormConfig().layout() == QgsEditFormConfig::TabLayout )
    {
      root = mLayer->editFormConfig().invisibleRootContainer();
    }
    else
    {
      root = generateRootContainer();
      mTemporaryContainer = root;
    }

    setHasTabs( !root->children().isEmpty() && QgsAttributeEditorElement::AeTypeContainer == root->children().first()->type() );
  }
  endResetModel();
}

void AttributeFormModel::onFeatureChanged()
{
  if ( mLayer )
  {
    emit featureChanged();
  }
}

QgsAttributeEditorContainer* AttributeFormModel::generateRootContainer() const
{
  QgsAttributeEditorContainer* root = new QgsAttributeEditorContainer( QString(), nullptr );
  QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    QgsAttributeEditorField* field = new QgsAttributeEditorField( fields.at( i ).displayName(), i, root );
    root->addChildElement( field );
  }
  return root;
}

QgsAttributeEditorContainer* AttributeFormModel::invisibleRootContainer() const
{
  return mTemporaryContainer ? mTemporaryContainer : mLayer->editFormConfig().invisibleRootContainer();
}

bool AttributeFormModel::hasTabs() const
{
  return mHasTabs;
}

void AttributeFormModel::setHasTabs( bool hasTabs )
{
  if ( hasTabs == mHasTabs )
    return;

  mHasTabs = hasTabs;
  emit hasTabsChanged();
}

void AttributeFormModel::save()
{
  mFeatureModel->save();
}

void AttributeFormModel::create()
{
  mFeatureModel->create();
}
