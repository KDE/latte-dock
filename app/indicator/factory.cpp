/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "factory.h"

// local
#include "../layouts/importer.h"

// Qt
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QProcess>
#include <QTemporaryDir>
#include <QTimer>
#include <QLatin1String>

// KDE
#include <KDirWatch>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNotification>
#include <KPluginMetaData>
#include <KArchive/KTar>
#include <KArchive/KZip>
#include <KArchive/KArchiveEntry>
#include <KArchive/KArchiveDirectory>
#include <KNewStuff3/KNS3/QtQuickDialogWrapper>

namespace Latte {
namespace Indicator {

Factory::Factory(QObject *parent)
    : QObject(parent)
{
    m_parentWidget = new QWidget();

    m_mainPaths = Latte::Layouts::Importer::standardPaths();

    for(int i=0; i<m_mainPaths.count(); ++i) {
        m_mainPaths[i] = m_mainPaths[i] + "/latte/indicators";
        discoverNewIndicators(m_mainPaths[i]);
    }

    //! track paths for changes
    for(const auto &dir : m_mainPaths) {
        KDirWatch::self()->addDir(dir);
    }

    connect(KDirWatch::self(), &KDirWatch::dirty, this, [ & ](const QString & path) {
        if (m_indicatorsPaths.contains(path)) {
            //! indicator updated
            reload(path);
        } else if (m_mainPaths.contains(path)){
            //! consider indicator addition
            discoverNewIndicators(path);
        }
    });

    connect(KDirWatch::self(), &KDirWatch::deleted, this, [ & ](const QString & path) {
        if (m_indicatorsPaths.contains(path)) {
            //! indicator removed
            removeIndicatorRecords(path);
        }
    });

    qDebug() << m_plugins["org.kde.latte.default"].name();
}

Factory::~Factory()
{
    m_parentWidget->deleteLater();
}

bool Factory::pluginExists(QString id) const
{
    return m_plugins.contains(id);
}

int Factory::customPluginsCount()
{
    return m_customPluginIds.count();
}

QStringList Factory::customPluginIds()
{
    return m_customPluginIds;
}

QStringList Factory::customPluginNames()
{
    return m_customPluginNames;
}

QStringList Factory::customLocalPluginIds()
{
    return m_customLocalPluginIds;
}

KPluginMetaData Factory::metadata(QString pluginId)
{
    if (m_plugins.contains(pluginId)) {
        return m_plugins[pluginId];
    }

    return KPluginMetaData();
}

void Factory::reload(const QString &indicatorPath)
{
    QString pluginChangedId;

    if (!indicatorPath.isEmpty() && indicatorPath != "." && indicatorPath != "..") {
        QString metadataFile = metadataFileAbsolutePath(indicatorPath);

        if(QFileInfo(metadataFile).exists()) {
            KPluginMetaData metadata = KPluginMetaData(metadataFile);

            if (metadataAreValid(metadata)) {
                pluginChangedId = metadata.pluginId();
                QString uiFile = indicatorPath + "/package/" + metadata.value("X-Latte-MainScript");

                if (!m_plugins.contains(metadata.pluginId())) {
                    m_plugins[metadata.pluginId()] = metadata;
                }

                if (QFileInfo(uiFile).exists()) {
                    m_pluginUiPaths[metadata.pluginId()] = QFileInfo(uiFile).absolutePath();
                }

                if ((metadata.pluginId() != "org.kde.latte.default")
                        && (metadata.pluginId() != "org.kde.latte.plasma")
                        && (metadata.pluginId() != "org.kde.latte.plasmatabstyle")) {

                    //! find correct alphabetical position
                    int newPos = -1;

                    if (!m_customPluginIds.contains(metadata.pluginId())) {
                        for (int i=0; i<m_customPluginNames.count(); ++i) {
                            if (QString::compare(metadata.name(), m_customPluginNames[i], Qt::CaseInsensitive)<=0) {
                                newPos = i;
                                break;
                            }
                        }
                    }

                    if (!m_customPluginIds.contains(metadata.pluginId())) {
                        if (newPos == -1) {
                            m_customPluginIds << metadata.pluginId();
                        } else {
                            m_customPluginIds.insert(newPos, metadata.pluginId());
                        }
                    }

                    if (!m_customPluginNames.contains(metadata.name())) {
                        if (newPos == -1) {
                            m_customPluginNames << metadata.name();
                        } else {
                            m_customPluginNames.insert(newPos, metadata.name());
                        }
                    }
                }

                if (indicatorPath.startsWith(QDir::homePath())) {
                    m_customLocalPluginIds << metadata.pluginId();
                }
            }

            qDebug() << " Indicator Package Loaded ::: " << metadata.name() << " [" << metadata.pluginId() << "]" << " - [" << indicatorPath <<"]";

            /*qDebug() << " Indicator value ::: " << metadata.pluginId();
                            qDebug() << " Indicator value ::: " << metadata.fileName();
                            qDebug() << " Indicator value ::: " << metadata.value("X-Latte-MainScript");
                            qDebug() << " Indicator value ::: " << metadata.value("X-Latte-ConfigUi");
                            qDebug() << " Indicator value ::: " << metadata.value("X-Latte-ConfigXml");*/
        }
    }

    if (!pluginChangedId.isEmpty()) {
        emit indicatorChanged(pluginChangedId);
    }
}

void Factory::discoverNewIndicators(const QString &main)
{
    if (!m_mainPaths.contains(main)) {
        return;
    }

    QDirIterator indicatorsDirs(main, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);

    while(indicatorsDirs.hasNext()){
        indicatorsDirs.next();
        QString iPath = indicatorsDirs.filePath();

        if (!m_indicatorsPaths.contains(iPath)) {
            m_indicatorsPaths << iPath;
            KDirWatch::self()->addDir(iPath);
            reload(iPath);
        }
    }
}

void Factory::removeIndicatorRecords(const QString &path)
{
    if (m_indicatorsPaths.contains(path)) {
        QString pluginId =  path.section('/',-1);
        m_plugins.remove(pluginId);
        m_pluginUiPaths.remove(pluginId);

        int pos = m_customPluginIds.indexOf(pluginId);

        m_customPluginIds.removeAt(pos);
        m_customPluginNames.removeAt(pos);
        m_customLocalPluginIds.removeAll(pluginId);

        m_indicatorsPaths.removeAll(path);

        KDirWatch::self()->removeDir(path);

        //! delay informing the removal in case it is just an update
        QTimer::singleShot(1000, [this, pluginId]() {
            emit indicatorRemoved(pluginId);
        });
    }
}

bool Factory::isCustomType(const QString &id) const
{
    return ((id != "org.kde.latte.default") && (id != "org.kde.latte.plasma") && (id != "org.kde.latte.plasmatabstyle"));
}

bool Factory::metadataAreValid(KPluginMetaData &metadata)
{
    return metadata.isValid()
            && metadata.category() == QLatin1String("Latte Indicator")
            && !metadata.value("X-Latte-MainScript").isEmpty();
}

bool Factory::metadataAreValid(QString &file)
{
    if (QFileInfo(file).exists()) {
        KPluginMetaData metadata(file);
        return metadata.isValid();
    }

    return false;
}

QString Factory::uiPath(QString pluginName) const
{
    if (!m_pluginUiPaths.contains(pluginName)) {
        return "";
    }

    return m_pluginUiPaths[pluginName];
}

QString Factory::metadataFileAbsolutePath(const QString &directoryPath)
{
    QString metadataFile = directoryPath + "/metadata.json";

    if(QFileInfo(metadataFile).exists()) {
        return metadataFile;
    }

    metadataFile = directoryPath + "/metadata.desktop";

    if(QFileInfo(metadataFile).exists()) {
        return metadataFile;
    }

    return QString();
}

Latte::ImportExport::State Factory::importIndicatorFile(QString compressedFile)
{
    auto showNotificationError = []() {
        auto notification = new KNotification("import-fail", KNotification::CloseOnTimeout);
        notification->setText(i18n("Failed to import indicator"));
        notification->sendEvent();
    };

    auto showNotificationSucceed = [](QString name, bool updated) {
        auto notification = new KNotification("import-done", KNotification::CloseOnTimeout);
        notification->setText(updated ? i18nc("indicator_name, imported updated","%1 indicator updated successfully", name) :
                                        i18nc("indicator_name, imported success","%1 indicator installed successfully", name));
        notification->sendEvent();
    };

    KArchive *archive;

    KZip *zipArchive = new KZip(compressedFile);
    zipArchive->open(QIODevice::ReadOnly);

    //! if the file isnt a zip archive
    if (!zipArchive->isOpen()) {
        delete zipArchive;

        KTar *tarArchive = new KTar(compressedFile, QStringLiteral("application/x-tar"));
        tarArchive->open(QIODevice::ReadOnly);

        if (!tarArchive->isOpen()) {
            delete tarArchive;
            showNotificationError();
            return Latte::ImportExport::FailedState;
        } else {
            archive = tarArchive;
        }
    } else {
        archive = zipArchive;
    }

    QTemporaryDir archiveTempDir;
    archive->directory()->copyTo(archiveTempDir.path());

    //metadata file
    QString packagePath = archiveTempDir.path();
    QString metadataFile = metadataFileAbsolutePath(archiveTempDir.path());

    if (!QFileInfo(metadataFile).exists()){
        QDirIterator iter(archiveTempDir.path(), QDir::Dirs | QDir::NoDotAndDotDot);

        while(iter.hasNext() ) {
            QString currentPath = iter.next();

            QString tempMetadata = metadataFileAbsolutePath(currentPath);

            if (QFileInfo(tempMetadata).exists()) {
                metadataFile = tempMetadata;
                packagePath = currentPath;
            }
        }
    }

    KPluginMetaData metadata = KPluginMetaData(metadataFile);

    if (metadataAreValid(metadata)) {
        QStringList standardPaths = Latte::Layouts::Importer::standardPaths();
        QString installPath = standardPaths[0] + "/latte/indicators/" + metadata.pluginId();

        bool updated{QDir(installPath).exists()};

        if (QDir(installPath).exists()) {
            QDir(installPath).removeRecursively();
        }

        QProcess process;
        process.start(QString("mv " +packagePath + " " + installPath));
        process.waitForFinished();
        QString output(process.readAllStandardOutput());

        showNotificationSucceed(metadata.name(), updated);
        return Latte::ImportExport::InstalledState;
    }

    showNotificationError();
    return Latte::ImportExport::FailedState;
}

void Factory::removeIndicator(QString id)
{
    if (m_plugins.contains(id)) {
        QString pluginName = m_plugins[id].name();

        QDialog* dialog = new QDialog(nullptr);
        dialog->setWindowTitle(i18n("Remove Indicator Confirmation"));
        dialog->setObjectName("warning");
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        auto buttonbox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);

        KMessageBox::createKMessageBox(dialog,
                                       buttonbox,
                                       QMessageBox::Question,
                                       i18n("Do you want to remove completely <b>%1</b> indicator from your system?", pluginName),
                                       QStringList(),
                                       QString(),
                                       0,
                                       KMessageBox::NoExec,
                                       QString());

        connect(buttonbox, &QDialogButtonBox::accepted, [&, id, pluginName]() {
            auto showRemovedSucceed = [](QString name) {
                auto notification = new KNotification("remove-done", KNotification::CloseOnTimeout);
                notification->setText(i18nc("indicator_name, removed success","<b>%1</b> indicator removed successfully", name));
                notification->sendEvent();
            };

            qDebug() << "Trying to remove indicator :: " << id;
            QProcess process;
            process.start(QString("kpackagetool5 -r " +id + " -t Latte/Indicator"));
            process.waitForFinished();
            showRemovedSucceed(pluginName);
        });

        dialog->show();
    }
}

void Factory::downloadIndicator()
{
    KNS3::QtQuickDialogWrapper dialog(QStringLiteral("latte-indicators.knsrc"), m_parentWidget);
    dialog.exec();
}

}
}
