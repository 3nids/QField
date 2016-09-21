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


AttributeFormModel::AttributeFormModel( QObject* parent )
  : QStandardItemModel( 0, 1, parent )
  , mFeatureModel( nullptr )
  , mLayer( nullptr )
  , mTemporaryContainer( nullptr )
{
}

AttributeFormModel::~AttributeFormModel()
{
  delete mTemporaryContainer;
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
  roles[Group] = "Group";

  return roles;
}

bool AttributeFormModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  if ( data( index, role ) != value )
  {
    switch ( role )
    {
      case RememberValue:
      {
        QStandardItem* item = itemFromIndex( index );
        int fieldIndex = item->data( FieldIndex ).toInt();
        bool changed = mFeatureModel->setData( mFeatureModel->index( fieldIndex ), value, FeatureModel::RememberAttribute );
        if ( changed )
        {
          item->setData( value, RememberValue );
          emit dataChanged( index, index, QVector<int>() << role );
        }
        return changed;
        break;
      }

      case AttributeValue:
      {
        QStandardItem* item = itemFromIndex( index );
        int fieldIndex = item->data( FieldIndex ).toInt();
        bool changed = mFeatureModel->setData( mFeatureModel->index( fieldIndex ), value, FeatureModel::AttributeValue );
        if ( changed )
        {
          item->setData( value, AttributeValue );
          emit dataChanged( index, index, QVector<int>() << role );
        }
        return changed;
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

  disconnect( mFeatureModel, &FeatureModel::currentLayerChanged, this, &AttributeFormModel::onLayerChanged );
  disconnect( mFeatureModel, &FeatureModel::featureChanged, this, &AttributeFormModel::onFeatureChanged );


  mFeatureModel = featureModel;

  connect( mFeatureModel, &FeatureModel::currentLayerChanged, this, &AttributeFormModel::onLayerChanged );
  connect( mFeatureModel, &FeatureModel::featureChanged, this, &AttributeFormModel::onFeatureChanged );

  emit featureModelChanged();
}

void AttributeFormModel::onLayerChanged()
{
  clear();

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

    invisibleRootItem()->setColumnCount( 1 );
    if ( mHasTabs )
    {
      Q_FOREACH( QgsAttributeEditorElement* element, root->children() )
      {
        QStandardItem* item = new QStandardItem();
        item->setData( element->name(), Name );
        item->setData( "container", ElementType );
        invisibleRootItem()->appendRow( item );
        flatten( static_cast<QgsAttributeEditorContainer*>( element ), item );
      }
    }
    else
    {
      flatten( invisibleRootContainer(), invisibleRootItem() );
    }
  }
}

void AttributeFormModel::onFeatureChanged()
{
  for ( int i = 0 ; i < invisibleRootItem()->rowCount(); ++i )
  {
    updateAttributeValue( invisibleRootItem()->child( i ) );
  }
  emit dataChanged( QModelIndex(), QModelIndex() );
}

QgsAttributeEditorContainer* AttributeFormModel::generateRootContainer() const
{
  QgsAttributeEditorContainer* root = new QgsAttributeEditorContainer( QString(), nullptr );
  QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    QgsAttributeEditorField* field = new QgsAttributeEditorField( fields.at( i ).name(), i, root );
    root->addChildElement( field );
  }
  return root;
}

QgsAttributeEditorContainer* AttributeFormModel::invisibleRootContainer() const
{
  return mTemporaryContainer ? mTemporaryContainer : mLayer->editFormConfig().invisibleRootContainer();
}

void AttributeFormModel::updateAttributeValue( QStandardItem* item )
{
  if ( item->data( ElementType ) == "field" )
  {
    item->setData( mFeatureModel->feature().attribute( item->data( FieldIndex ).toInt() ), AttributeValue );
    emit dataChanged( item->index(), item->index(), QVector<int>() << AttributeValue );
  }
  else
    for ( int i = 0; i < item->rowCount(); ++i )
      updateAttributeValue( item->child( i ) );
}

void AttributeFormModel::flatten( QgsAttributeEditorContainer* container, QStandardItem* parent )
{
  Q_FOREACH( QgsAttributeEditorElement* element, container->children() )
  {
    switch ( element->type() )
    {
      case QgsAttributeEditorElement::AeTypeContainer:
        flatten( static_cast<QgsAttributeEditorContainer*>( element ), parent );
        break;

      case QgsAttributeEditorElement::AeTypeField:
      {
        QgsAttributeEditorField* editorField = static_cast<QgsAttributeEditorField*>( element );
        if ( editorField->idx() == -1 )
          continue;

        QStandardItem* item = new QStandardItem();

        item->setData( mLayer->attributeDisplayName( editorField->idx() ), Name );
        item->setData( mFeatureModel->feature().attribute( editorField->idx() ), AttributeValue );
        item->setData( mLayer->editFormConfig().readOnly( editorField->idx() ), AttributeEditable );
        item->setData( mLayer->editFormConfig().widgetType( editorField->name() ), EditorWidget );
        item->setData( mLayer->editFormConfig().widgetConfig( editorField->name() ), EditorWidgetConfig );
        item->setData( mFeatureModel->rememberedAttributes().at( editorField->idx() ) ? Qt::Checked : Qt::Unchecked, RememberValue );
        item->setData( mLayer->fields().at( editorField->idx() ), Field );
        item->setData( "field", ElementType );
        item->setData( editorField->idx(), FieldIndex );
        item->setData( container->isGroupBox() ? container->name() : QString(), Group );

        parent->appendRow( item );
        break;
      }
      case QgsAttributeEditorElement::AeTypeRelation:
        // todo
        break;
      case QgsAttributeEditorElement::AeTypeInvalid:
        // todo
        break;
    }
  }
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
