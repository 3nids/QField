/***************************************************************************
    qfieldcloudconnection.cpp
    ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qfield.h"
#include "qfieldcloudconnection.h"
#include <qgsnetworkaccessmanager.h>
#include <qgsapplication.h>
#include <qgsmessagelog.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QHttpMultiPart>
#include <QSettings>
#include <QTimer>
#include <QUrlQuery>
#include <QFile>


QFieldCloudConnection::QFieldCloudConnection()
  : mUrl( QSettings().value( QStringLiteral( "/QFieldCloud/url" ), defaultUrl() ).toString() )
  , mToken( QSettings().value( QStringLiteral( "/QFieldCloud/token" ) ).toByteArray() )
{
  QgsNetworkAccessManager::instance()->setTimeout( 10 * 60 * 1000 );
}

QString QFieldCloudConnection::errorString( QNetworkReply *reply )
{
  if ( !reply )
    return QString();

  if ( reply->error() == QNetworkReply::NoError )
    return QString();

  QJsonParseError jsonError;
  const QJsonObject doc = QJsonDocument::fromJson( reply->readAll(), &jsonError ).object();

  QString errorMessage;
  if ( jsonError.error == QJsonParseError::NoError )
  {
    if ( doc.contains( QStringLiteral( "detail" ) ) )
      errorMessage += doc.value( QStringLiteral( "detail" ) ).toString();
    else
      errorMessage += QStringLiteral( "<no server details>" );

    if ( errorMessage.isEmpty() )
      errorMessage += QStringLiteral( "<empty server details>" );
  }
  else
  {
    errorMessage += tr( "Cannot read JSON from failed request: %1. " ).arg( jsonError.errorString() );
  }

  int httpCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  errorMessage += QStringLiteral( "[HTTP/%1] %2 " ).arg( httpCode ).arg( reply->errorString() );

  errorMessage += ( httpCode > 400 )
      ? tr( "Server Error." )
      : tr( "Network Error." );

  return errorMessage;
}

QString QFieldCloudConnection::url() const
{
  return mUrl;
}

void QFieldCloudConnection::setUrl( const QString &url )
{
  if ( url == mUrl )
    return;

  mUrl = url;

  QSettings().setValue( QStringLiteral( "/QFieldCloud/url" ), url );

  emit urlChanged();
}

QString QFieldCloudConnection::defaultUrl()
{
  return QStringLiteral( "https://app.qfield.cloud" );
}

QString QFieldCloudConnection::username() const
{
  return mUsername;
}

void QFieldCloudConnection::setUsername( const QString &username )
{
  if ( mUsername == username )
    return;

  mUsername = username;
  invalidateToken();

  emit usernameChanged();
}

QString QFieldCloudConnection::password() const
{
  return mPassword;
}

void QFieldCloudConnection::setPassword( const QString &password )
{
  if ( password == mPassword )
    return;

  mPassword = password;
  emit passwordChanged();

}

void QFieldCloudConnection::login()
{
  NetworkReply *reply = ( ! mToken.isEmpty() && ( mPassword.isEmpty() || mUsername.isEmpty() ) )
        ? get( QStringLiteral( "/api/v1/auth/user/" ) )
        : post( QStringLiteral( "/api/v1/auth/token/" ), QVariantMap({
                         {"username", mUsername},
                         {"password", mPassword},
                       }) );

  setStatus( ConnectionStatus::Connecting );

  connect( reply, &NetworkReply::finished, this, [ = ]()
  {
    QNetworkReply *rawReply = reply->reply();

    Q_ASSERT( reply->isFinished() );
    Q_ASSERT( rawReply );

    reply->deleteLater();
    rawReply->deleteLater();

    if ( rawReply->error() != QNetworkReply::NoError )
    {
      int httpCode = rawReply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();

      if ( rawReply->error() == QNetworkReply::HostNotFoundError )
      {
        emit loginFailed( tr( "Server not found, please check the server URL" ) );
      }
      else if ( rawReply->error() == QNetworkReply::TimeoutError )
      {
        emit loginFailed( tr( "Timeout error, please retry" ) );
      }
      else if ( httpCode == 400 )
      {
        emit loginFailed( tr( "Wrong username or password" ) );
      }
      else
      {
        QString message( errorString( rawReply ) );
        emit loginFailed( message );
        QgsMessageLog::logMessage( message );
      }
      setStatus( ConnectionStatus::Disconnected );
      return;
    }

    QJsonObject resp = QJsonDocument::fromJson( rawReply->readAll() ).object();

    if ( resp.isEmpty() )
    {
      emit loginFailed( tr( "Login temporary unavailable" ) );
      setStatus( ConnectionStatus::Disconnected );
      return;
    }

    QByteArray token = resp.value( QStringLiteral( "token" ) ).toString().toUtf8();

    if ( !token.isEmpty() )
    {
      setToken( token );
    }

    mUsername  = resp.value( QStringLiteral( "username" ) ).toString();
    setStatus( ConnectionStatus::LoggedIn );

  } );
}

void QFieldCloudConnection::logout()
{
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
  QNetworkRequest request( mUrl + QStringLiteral( "/api/v1/auth/logout/" ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
  setAuthenticationToken( request );

  QNetworkReply *reply = nam->post( request, QByteArray() );

  connect( reply, &QNetworkReply::finished, this, [reply]()
  {
    reply->deleteLater();
  } );

  mPassword.clear();
  invalidateToken();
  QSettings().remove( "/QFieldCloud/token" );

  setStatus( ConnectionStatus::Disconnected );
}

QFieldCloudConnection::ConnectionStatus QFieldCloudConnection::status() const
{
  return mStatus;
}

QFieldCloudConnection::ConnectionState QFieldCloudConnection::state() const
{
  return mState;
}

NetworkReply *QFieldCloudConnection::post( const QString &endpoint, const QVariantMap &params, const QStringList &fileNames )
{
  QNetworkRequest request( mUrl + endpoint );
  QByteArray requestBody = QJsonDocument( QJsonObject::fromVariantMap( params ) ).toJson();
  setAuthenticationToken( request );
  request.setAttribute( QNetworkRequest::FollowRedirectsAttribute, true );

  if ( fileNames.isEmpty() )
  {
    request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );

    return NetworkManager::post( request, requestBody );
  }

  QHttpMultiPart *multiPart = new QHttpMultiPart( QHttpMultiPart::FormDataType );
  QHttpPart textPart;

  QJsonDocument doc( QJsonObject::fromVariantMap( params ) );
  textPart.setHeader( QNetworkRequest::ContentTypeHeader, QVariant( "application/json" ) );
  textPart.setHeader( QNetworkRequest::ContentDispositionHeader, QVariant( "form-data; name=\"text\"" ) );
  textPart.setBody( doc.toJson() );

  multiPart->append( textPart );

  for ( const QString &fileName : fileNames )
  {
    QHttpPart filePart;
    QFile *file = new QFile( fileName, multiPart );

    if ( ! file->open( QIODevice::ReadOnly ) )
      return nullptr;

    const QString header = QStringLiteral( "form-data; name=\"file\"; filename=\"%1\"" ).arg( fileName );
    filePart.setHeader( QNetworkRequest::ContentTypeHeader, QVariant( "application/json" ) );
    filePart.setHeader( QNetworkRequest::ContentDispositionHeader, header );
    filePart.setBodyDevice( file );

    multiPart->append( filePart );
  }

  NetworkReply *reply = NetworkManager::post( request, multiPart );

  multiPart->setParent( reply );

  mPendingRequests++;
  setState( ConnectionState::Busy );
  connect( reply, &NetworkReply::finished, this, [=]() { if ( --mPendingRequests == 0 ) setState( ConnectionState::Idle ); } );

  return reply;
}

NetworkReply *QFieldCloudConnection::get( const QString &endpoint, const QVariantMap &params )
{
  QNetworkRequest request;
  QUrl url( mUrl + endpoint );
  QUrlQuery urlQuery;

  for ( auto [ key, value ] : qfield::asKeyValueRange( params ) )
    urlQuery.addQueryItem( key, value.toString() );

  url.setQuery( urlQuery );

  request.setUrl( url );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
  request.setAttribute( QNetworkRequest::FollowRedirectsAttribute, true );
  setAuthenticationToken( request );

  NetworkReply *reply = NetworkManager::get( request );

  mPendingRequests++;
  setState( ConnectionState::Busy );
  connect( reply, &NetworkReply::finished, this, [=]() { if ( --mPendingRequests == 0 ) setState( ConnectionState::Idle ); } );

  return reply;
}

void QFieldCloudConnection::setToken( const QByteArray &token )
{
  if ( mToken == token )
    return;

  mToken = token;
  QSettings().setValue( "/QFieldCloud/token", token );

  emit tokenChanged();
}

void QFieldCloudConnection::invalidateToken()
{
  if ( mToken.isNull() )
    return;

  mToken = QByteArray();
  emit tokenChanged();
}

void QFieldCloudConnection::setStatus( ConnectionStatus status )
{
  if ( mStatus == status )
    return;

  mStatus = status;
  emit statusChanged();
}

void QFieldCloudConnection::setState( ConnectionState state )
{
  if ( mState == state )
    return;

  mState = state;
  emit stateChanged();
}

void QFieldCloudConnection::setAuthenticationToken( QNetworkRequest &request )
{
  if ( !mToken.isNull() )
  {
    request.setRawHeader( "Authorization", "Token " + mToken );
  }
}

