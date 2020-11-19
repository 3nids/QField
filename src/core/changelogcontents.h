/***************************************************************************
  changelogcontents.h - Changelog

 ---------------------
 begin                : Nov 2020
 copyright            : (C) 2020 by Ivan Ivanov
 email                : ivan@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef CHANGELOGCONTENTS_H
#define CHANGELOGCONTENTS_H

#include <QObject>

class ChangelogContents : public QObject
{
    Q_OBJECT

  public:
    explicit ChangelogContents( QObject *parent = nullptr );

    Q_INVOKABLE void request();

    Q_INVOKABLE QString markdown();

  signals:
    void changelogFetchFinished( bool isSuccess );

  private:
    QList<int> parseVersion( const QString &version );

    QString mMarkdown;
};

#endif // CHANGELOGCONTENTS_H
