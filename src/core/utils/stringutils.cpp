/***************************************************************************
                        stringutils.cpp
                        ---------------
  begin                : Jun 2020
  copyright            : (C) 2020 by Ivan Ivanov
  email                : ivan@opengis.ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "stringutils.h"

#include "qgsstringutils.h"


#include <QUrl>


StringUtils::StringUtils( QObject *parent )
  : QObject( parent )
{

}


bool StringUtils::isRelativeUrl( const QString &url )
{
  return QUrl( url ).isRelative();
}


QString StringUtils::insertLinks( const QString &string )
{
  return QgsStringUtils::insertLinks( string );
}
