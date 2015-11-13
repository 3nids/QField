/***************************************************************************
              featurelistmodelhighlight.cpp
               ----------------------------------------------------
              date                 : 9.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "featurelistmodelhighlight.h"

#include "qgssggeometry.h"

FeatureListModelHighlight::FeatureListModelHighlight( QQuickItem* parent )
  : QQuickItem( parent )
  , mModel( 0 )
  , mSelection( 0 )
  , mDirty( false )
{
  setFlags( QQuickItem::ItemHasContents );
  setAntialiasing( true );

  connect( this, SIGNAL( modelChanged() ), this, SLOT( onDataChanged() ) );
  connect( this, SIGNAL( selectionChanged() ), this, SLOT( onDataChanged() ) );
}

void FeatureListModelHighlight::setSelection( FeatureListModelSelection* selection )
{
  if ( selection != mSelection )
  {
    disconnect( this, SLOT( onSelectionChanged() ) );
    mSelection = selection;
    connect( selection, SIGNAL( selectionChanged() ), this, SLOT( onSelectionChanged() ) );
    emit selectionChanged();
  }
}

FeatureListModelSelection* FeatureListModelHighlight::selection() const
{
  return mSelection;
}

void FeatureListModelHighlight::onDataChanged()
{
  if ( mModel )
  {
    connect( mModel, SIGNAL( modelReset() ), this, SLOT( onModelDataChanged() ) );
  }
}

void FeatureListModelHighlight::onModelDataChanged()
{
  mDirty = true;
  update();
}

void FeatureListModelHighlight::onSelectionChanged()
{
  mDirty = true;
  update();
}

QSGNode* FeatureListModelHighlight::updatePaintNode( QSGNode* n, QQuickItem::UpdatePaintNodeData* )
{
  if ( mDirty )
  {
    delete n;
    n = new QSGNode;

    int count = mModel->rowCount( QModelIndex() );
    QgsSGGeometry* sn = 0;

    for ( int i = 0; i < count; ++i )
    {
      QgsSGGeometry* gn;

      QModelIndex index = mModel->index( i, 0, QModelIndex() );
      const Feature& feature = mModel->data( index, FeatureListModel::FeatureRole ).value<Feature>();

      if ( mSelection && mSelection->selection() == i )
      {
        sn = new QgsSGGeometry( *feature.qgsFeature().constGeometry(), mSelectionColor, mWidth );
        sn->setFlag( QSGNode::OwnedByParent );
      }
      else
      {
        gn = new QgsSGGeometry( *feature.qgsFeature().constGeometry(), mColor, mWidth );
        gn->setFlag( QSGNode::OwnedByParent );
        n->appendChildNode( gn );
      }
    }

    if ( sn )
      n->appendChildNode( sn );

    mDirty = false;
  }

  return n;
}
