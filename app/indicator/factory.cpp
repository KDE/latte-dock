/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "factory.h"

// local
#include "../importer.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QTemporaryDir>

// KDE
#include <KLocalizedString>
#include <KNotification>
#include <KPluginMetaData>
#include <KArchive/KTar>
#include <KArchive/KZip>
#include <KArchive/KArchiveEntry>
#include <KArchive/KArchiveDirectory>
#include <KNewStuff3/KNS3/DownloadDialog>

namespace Latte {
namespace Indicator {

Factory::Factory(QObject *parent)
    : QObject(parent)
{
    m_parentWidget = new QWidget();
    reload();

    qDebug() << m_plugins["org.kde.latte.default"].name();
}

Factory::~Factory()
{
    m_parentWidget->deleteLater();
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


KPluginMetaData Factory::metadata(QString pluginId)
{
    if (m_plugins.contains(pluginId)) {
        return m_plugins[pluginId];
    }

    return KPluginMetaData();
}

void Factory::reload()
{
    QStringList standardPaths = Latte::Importer::standardPaths();

    m_plugins.clear();
    m_customPluginIds.clear();
    m_customPluginNames.clear();

    foreach(auto path, standardPaths) {
        QDir standard(path + "/latte/indicators");

        if (standard.exists()) {
            QStringList pluginDirs = standard.entryList(QStringList(),QDir::AllDirs | QDir::NoSymLinks);

            foreach (auto pluginDir, pluginDirs) {
                if (pluginDir != "." && pluginDir != "..") {
                    QString metadataFile = standard.absolutePath() + "/" + pluginDir + "/metadata.desktop";
                    KPluginMetaData metadata = KPluginMetaData::fromDesktopFile(metadataFile);

                    if (metadataAreValid(metadata)) {
                        QString uiFile = standard.absolutePath() + "/" + pluginDir + "/package/" + metadata.value("X-Latte-MainScript");

                        if (QFileInfo(uiFile).exists() && !m_plugins.contains(metadata.pluginId())) {
                            m_plugins[metadata.pluginId()] = metadata;

                            if ((metadata.pluginId() != "org.kde.latte.default")
                                    && (metadata.pluginId() != "org.kde.latte.plasma")) {
                                m_customPluginIds << metadata.pluginId();
                                m_customPluginNames << metadata.name();
                            }

                            QString pluginPath = metadata.fileName().remove("metadata.desktop");
                            qDebug() << " Indicator Package Loaded ::: " << metadata.name() << " [" << metadata.pluginId() << "]" << " - [" <<pluginPath<<"]";
                            /*qDebug() << " Indicator value ::: " << metadata.pluginId();
                            qDebug() << " Indicator value ::: " << metadata.fileName();
                            qDebug() << " Indicator value ::: " << metadata.value("X-Latte-MainScript");
                            qDebug() << " Indicator value ::: " << metadata.value("X-Latte-ConfigUi");
                            qDebug() << " Indicator value ::: " << metadata.value("X-Latte-ConfigXml");*/
                        }
                    }
                }
            }
        }
    }

    emit customPluginsChanged();
}

bool Factory::metadataAreValid(KPluginMetaData &metadata)
{
    return metadata.isValid()
            && metadata.category() == "Latte Indicator"
            && !metadata.value("X-Latte-MainScript").isEmpty();
}

bool Factory::metadataAreValid(QString &file)
{
    if (QFileInfo(file).exists()) {
        KPluginMetaData metadata = KPluginMetaData::fromDesktopFile(file);
        return metadataAreValid(metadata);
    }

    return false;
}


Latte::Types::ImportExportState Factory::importIndicatorFile(QString compressedFile)
{
    auto showNotificationError = []() {
        auto notification = new KNotification("import-fail", KNotification::CloseOnTimeout);
        notification->setText(i18n("Failed to import indicator"));
        notification->sendEvent();
    };

    auto showNotificationSucceed = [](QString name, bool updated) {
        auto notification = new KNotification("import-done", KNotification::CloseOnTimeout);
        notification->setText(updated ? i18nc("indicator_name, imported updated","%0 indicator updated successfully").arg(name) :
                                        i18nc("indicator_name, imported success","%0 indicator installed successfully").arg(name));
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
            return Latte::Types::Failed;
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
    QString metadataFile = archiveTempDir.path() + "/metadata.desktop";

    if (!QFileInfo(metadataFile).exists()){
        QDirIterator iter(archiveTempDir.path(), QDir::Dirs | QDir::NoDotAndDotDot);

        while(iter.hasNext() ) {
            QString currentPath = iter.next();

            QString tempMetadata = currentPath + "/metadata.desktop";

            if (QFileInfo(tempMetadata).exists()) {
                metadataFile = tempMetadata;
                packagePath = currentPath;
            }
        }
    }

    KPluginMetaData metadata = KPluginMetaData::fromDesktopFile(metadataFile);

    if (metadataAreValid(metadata)) {
        QStringList standardPaths = Latte::Importer::standardPaths();
        QString installPath = standardPaths[0] + "/latte/indicators/" + metadata.pluginId();

        bool updated{QDir(installPath).exists()};

        if (QDir(installPath).exists()) {
            QDir(installPath).removeRecursively();
        }

        //! Identify Plasma Desktop version
        QProcess process;
        process.start(QString("mv " +packagePath + " " + installPath));
        process.waitForFinished();
        QString output(process.readAllStandardOutput());

        showNotificationSucceed(metadata.name(), updated);
        return Latte::Types::Installed;
    }

    showNotificationError();
    return Latte::Types::Failed;
}

void Factory::downloadIndicator()
{
    KNS3::DownloadDialog dialog(QStringLiteral("latte-indicators.knsrc"), m_parentWidget);

    dialog.exec();

    bool layoutAdded{false};

    if (!dialog.changedEntries().isEmpty() || !dialog.installedEntries().isEmpty()) {
        reload();
    }
}

}
}
