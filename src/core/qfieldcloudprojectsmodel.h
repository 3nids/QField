/***************************************************************************
    qfieldcloudprojectsmodel.h
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

#ifndef QFIELDCLOUDPROJECTSMODEL_H
#define QFIELDCLOUDPROJECTSMODEL_H

#include "qgsnetworkaccessmanager.h"
#include "qgsgpkgflusher.h"
#include "deltastatuslistmodel.h"

#include <QAbstractListModel>
#include <QNetworkReply>
#include <QTimer>


class QNetworkRequest;
class QFieldCloudConnection;
class NetworkReply;
class LayerObserver;
class QgsMapLayer;
class QgsProject;


class QFieldCloudProjectsModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    enum ColumnRole
    {
      IdRole = Qt::UserRole + 1,
      OwnerRole,
      NameRole,
      DescriptionRole,
      ModificationRole,
      CheckoutRole,
      StatusRole,
      ErrorStatusRole,
      ErrorStringRole,
      DownloadProgressRole,
      ExportStatusRole,
      UploadAttachmentsProgressRole,
      UploadDeltaProgressRole,
      UploadDeltaStatusRole,
      UploadDeltaStatusStringRole,
      LocalDeltasCountRole,
      LocalPathRole,
      CanSyncRole,
    };

    Q_ENUM( ColumnRole )

    //! Whether the project is busy or idle.
    enum class ProjectStatus
    {
      Idle,
      Downloading,
      Uploading,
    };

    Q_ENUM( ProjectStatus )

    //! Whether the project has experienced an error.
    enum ProjectErrorStatus
    {
      NoErrorStatus,
      DownloadErrorStatus,
      UploadErrorStatus,
    };

    Q_ENUM( ProjectErrorStatus )

    //! Whether the project has been available locally and/or remotely.
    enum ProjectCheckout
    {
      RemoteCheckout = 2 << 0,
      LocalCheckout = 2 << 1,
      LocalAndRemoteCheckout = RemoteCheckout | LocalCheckout
    };

    Q_ENUM( ProjectCheckout )
    Q_DECLARE_FLAGS( ProjectCheckouts, ProjectCheckout )
    Q_FLAG( ProjectCheckouts )

    //! Whether the project has no or local and/or remote modification. Indicates wheter can be synced.
    enum ProjectModification
    {
      NoModification = 0,
      LocalModification = 2 << 0,
      RemoteModification = 2 << 1,
      LocalAndRemoteModification = RemoteModification | LocalModification
    };

    Q_ENUM( ProjectModification )
    Q_DECLARE_FLAGS( ProjectModifications, ProjectModification )
    Q_FLAG( ProjectModifications )

    //! The status of the running server job for applying deltas on a project.
    enum DeltaFileStatus
    {
      DeltaFileErrorStatus,
      DeltaFileLocalStatus,
      DeltaFilePendingStatus,
      DeltaFileAppliedStatus,
    };

    Q_ENUM( DeltaFileStatus )

    //! The status of the running server job for exporting a project.
    enum ExportStatus
    {
      ExportErrorStatus,
      ExportUnstartedStatus,
      ExportPendingStatus,
      ExportBusyStatus,
      ExportFinishedStatus,
    };

    Q_ENUM( ExportStatus )

    QFieldCloudProjectsModel();

    //! Stores the current cloud connection.
    Q_PROPERTY( QFieldCloudConnection *cloudConnection READ cloudConnection WRITE setCloudConnection NOTIFY cloudConnectionChanged )

    //! Returns the currently used cloud connection.
    QFieldCloudConnection *cloudConnection() const;

    //! Sets the cloud connection.
    void setCloudConnection( QFieldCloudConnection *cloudConnection );

    //! Stores the current layer observer.
    Q_PROPERTY( LayerObserver *layerObserver READ layerObserver WRITE setLayerObserver NOTIFY layerObserverChanged )

    //! Returns the currently used layer observer.
    LayerObserver *layerObserver() const;

    //! Sets the layer observer.
    void setLayerObserver( LayerObserver *layerObserver );

    //! Stores the cloud project id of the currently opened project. Empty string if missing.
    Q_PROPERTY( QString currentProjectId READ currentProjectId WRITE setCurrentProjectId NOTIFY currentProjectIdChanged )

    //! Returns the cloud project id of the currently opened project.
    QString currentProjectId() const;

    //! Sets the cloud project id of the currently opened project.
    void setCurrentProjectId( const QString &currentProjectId );

    //! Stores the geopackage flusher, write only
    Q_PROPERTY( QgsGpkgFlusher *gpkgFlusher WRITE setGpkgFlusher NOTIFY gpkgFlusherChanged )

    //! Sets the geopackage flusher
    void setGpkgFlusher( QgsGpkgFlusher *flusher );

    //! Stores the cloud project data of the currently opened project. Empty map if missing.
    Q_PROPERTY( QVariant currentProjectData READ currentProjectData NOTIFY currentProjectDataChanged )

    //! Returns the cloud project data of the currently opened project.
    QVariantMap currentProjectData() const;

    //! Returns the cloud project data for given \a projectId.
    Q_INVOKABLE QVariantMap getProjectData( const QString &projectId ) const;

    //! Requests the cloud projects list from the server.
    Q_INVOKABLE void refreshProjectsList();

    //! Downloads a cloud project with given \a projectId and all of its files.
    Q_INVOKABLE void downloadProject( const QString &projectId );

    //! Pushes all local deltas for given \a projectId. If \a shouldDownloadUpdates is true, also calls `downloadProject`.
    Q_INVOKABLE void uploadProject( const QString &projectId, const bool shouldDownloadUpdates );

    //! Remove local cloud project with given \a projectId from the device storage
    Q_INVOKABLE void removeLocalProject( const QString &projectId );

    //! Reverts the deltas of the current cloud project. The changes would applied in reverse order and opposite methods, e.g. "delete" becomes "create".
    Q_INVOKABLE bool revertLocalChangesFromCurrentProject();

    //! Discards the delta records of the current cloud project.
    Q_INVOKABLE bool discardLocalChangesFromCurrentProject();

    //! Returns the cloud project status for given \a projectId.
    Q_INVOKABLE ProjectStatus projectStatus( const QString &projectId ) const;

    //! Returns the cloud project modification for given \a projectId.
    Q_INVOKABLE ProjectModifications projectModification( const QString &projectId ) const;

    //! Updates the project modification for given \a projectId.
    Q_INVOKABLE void refreshProjectModification( const QString &projectId );

    //! Returns the model role names.
    QHash<int, QByteArray> roleNames() const override;

    //! Returns number of rows.
    int rowCount( const QModelIndex &parent ) const override;

    //! Returns the data at given \a index with given \a role.
    QVariant data( const QModelIndex &index, int role ) const override;

    //! Reloads the list of cloud projects with the given list of \a remoteProjects.
    Q_INVOKABLE void reload( const QJsonArray &remoteProjects );

  signals:
    void cloudConnectionChanged();
    void layerObserverChanged();
    void currentProjectIdChanged();
    void currentProjectDataChanged();
    void canSyncCurrentProjectChanged();
    void gpkgFlusherChanged();
    void warning( const QString &message );
    void projectDownloaded( const QString &projectId, const bool hasError, const QString &projectName );
    void projectStatusChanged( const QString &projectId, const ProjectStatus &projectStatus );

    //
    void networkDeltaUploaded( const QString &projectId );
    void networkDeltaStatusChecked( const QString &projectId );
    void networkAttachmentsUploaded( const QString &projectId );
    void networkAllAttachmentsUploaded( const QString &projectId );
    void networkLayerDownloaded( const QString &projectId );
    void networkAllLayersDownloaded( const QString &projectId );
    void pushFinished( const QString &projectId, bool hasError, const QString &errorString = QString() );
    void downloadFinished( const QString &projectId, bool hasError, const QString &errorString = QString() );

  private slots:
    void connectionStatusChanged();
    void projectListReceived();

    NetworkReply *uploadFile( const QString &projectId, const QString &fileName );

    int findProject( const QString &projectId ) const;

    void layerObserverLayerEdited( const QString &layerId );
  private:
    static const int sDelayBeforeStatusRetry = 1000;

    struct FileTransfer
    {
      FileTransfer(
        const QString &fileName,
        const int bytesTotal,
        NetworkReply *networkReply = nullptr,
        const QStringList &layerIds = QStringList()
      )
        : fileName( fileName ),
          bytesTotal( bytesTotal ),
          networkReply( networkReply ),
          layerIds( layerIds )
      {};

      FileTransfer() = default;

      QString fileName;
      QString tmpFile;
      int bytesTotal;
      int bytesTransferred = 0;
      bool isFinished = false;
      NetworkReply *networkReply;
      QNetworkReply::NetworkError error = QNetworkReply::NoError;
      QStringList layerIds;
    };

    struct CloudProject
    {
      CloudProject( const QString &id, const QString &owner, const QString &name, const QString &description, const QString &updatedAt, const ProjectCheckouts &checkout, const ProjectStatus &status )
        : id( id )
        , owner( owner )
        , name( name )
        , description( description )
        , updatedAt( updatedAt )
        , status( status )
        , checkout( checkout )
      {}

      CloudProject() = default;

      QString id;
      QString owner;
      QString name;
      QString description;
      QString updatedAt;
      ProjectStatus status;
      ProjectErrorStatus errorStatus = ProjectErrorStatus::NoErrorStatus;
      ProjectCheckouts checkout;
      ProjectModifications modification = ProjectModification::NoModification;
      QString localPath;

      QString deltaFileId;
      DeltaFileStatus deltaFileUploadStatus = DeltaFileLocalStatus;
      QString deltaFileUploadStatusString;
      QStringList deltaLayersToDownload;

      ExportStatus exportStatus = ExportUnstartedStatus;
      QString exportStatusString;
      QMap<QString, FileTransfer> downloadFileTransfers;
      int downloadFilesFinished = 0;
      int downloadFilesFailed = 0;
      int downloadBytesTotal = 0;
      int downloadBytesReceived = 0;
      double downloadProgress = 0.0; // range from 0.0 to 1.0

      QMap<QString, FileTransfer> uploadAttachments;
      int uploadAttachmentsFinished = 0;
      int uploadAttachmentsFailed = 0;
      int uploadAttachmentsBytesTotal = 0;
      double uploadAttachmentsProgress = 0.0; // range from 0.0 to 1.0
      double uploadDeltaProgress = 0.0; // range from 0.0 to 1.0

      int deltasCount = 0;
    };

    inline QString layerFileName( const QgsMapLayer *layer ) const;

    QList<CloudProject> mCloudProjects;
    QFieldCloudConnection *mCloudConnection = nullptr;
    QString mCurrentProjectId;
    LayerObserver *mLayerObserver = nullptr;
    QgsProject *mProject = nullptr;
    QgsGpkgFlusher *mGpkgFlusher = nullptr;

    std::unique_ptr<DeltaStatusListModel> mDeltaStatusListModel;

    void projectCancelUpload( const QString &projectId );
    void projectUploadAttachments( const QString &projectId );
    void projectGetDeltaStatus( const QString &projectId );
    void projectGetDownloadStatus( const QString &projectId );
    void projectDownloadLayers( const QString &projectId );
    bool projectMoveDownloadedFilesToPermanentStorage( const QString &projectId );

    NetworkReply *downloadFile( const QString &projectId, const QString &fileName );
    void projectDownloadFiles( const QString &projectId );

    bool canSyncProject( const QString &projectId ) const;

    bool deleteGpkgShmAndWal( const QStringList &gpkgFileNames );
    QStringList projectFileNames( const QString &projectPath, const QStringList &fileNames ) const;
    QStringList filterGpkgFileNames( const QStringList &fileNames ) const;

    QFieldCloudProjectsModel::ExportStatus exportStatus( const QString &status ) const;
};

Q_DECLARE_METATYPE( QFieldCloudProjectsModel::ProjectStatus )
Q_DECLARE_METATYPE( QFieldCloudProjectsModel::ProjectCheckout )
Q_DECLARE_METATYPE( QFieldCloudProjectsModel::ProjectCheckouts )
Q_DECLARE_OPERATORS_FOR_FLAGS( QFieldCloudProjectsModel::ProjectCheckouts )
Q_DECLARE_METATYPE( QFieldCloudProjectsModel::ProjectModification )
Q_DECLARE_METATYPE( QFieldCloudProjectsModel::ProjectModifications )
Q_DECLARE_OPERATORS_FOR_FLAGS( QFieldCloudProjectsModel::ProjectModifications )

#endif // QFIELDCLOUDPROJECTSMODEL_H
