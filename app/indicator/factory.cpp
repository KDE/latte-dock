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
#include "../layouts/importer.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QProcess>
#include <QTemporaryDir>

// KDE
#include <KDirWatch>
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

    m_watchedPaths = Latte::Layouts::Importer::standardPaths();

    for(int i=0; i<m_watchedPaths.count(); ++i) {
        m_watchedPaths[i] = m_watchedPaths[i] + "/latte/indicators";
    }

    reload();

    //! track paths for changes
    for(const auto &dir : m_watchedPaths) {
        KDirWatch::self()->addDir(dir);
    }

    connect(KDirWatch::self(), &KDirWatch::dirty, this, [ & ](const QString & path) {
        if (m_watchedPaths.contains(path)) {
            reload();
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

void Factory::reload()
{
    m_plugins.clear();
    m_customPluginIds.clear();
    m_customPluginNames.clear();
    m_customLocalPluginIds.clear();

    for(const auto &path : m_watchedPaths) {
        QDir standard(path);

        if (standard.exists()) {
            QStringList pluginDirs = standard.entryList(QStringList(),QDir::AllDirs | QDir::NoSymLinks);

            for (const auto &pluginDir : pluginDirs) {
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

                            if (standard.absolutePath().startsWith(QDir::homePath())) {
                                m_customLocalPluginIds << metadata.pluginId();
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
        return metadata.isValid();
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
        return Latte::Types::Installed;
    }

    showNotificationError();
    return Latte::Types::Failed;
}

void Factory::removeIndicator(QString id)
{
    if (m_plugins.contains(id)) {
        QString pluginName = m_plugins[id].name();

        auto msg = new QMessageBox(m_parentWidget);
        msg->setIcon(QMessageBox::Warning);
        msg->setWindowTitle(i18n("Remove Indicator"));
        msg->setText(
                    i18n("Do you want to remove <b>%0</b> indicator from your system?").arg(pluginName));
        msg->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg->setDefaultButton(QMessageBox::No);

        connect(msg, &QMessageBox::finished, this, [ &, msg, id, pluginName](int result) {
            auto showRemovedSucceed = [](QString name) {
                auto notification = new KNotification("remove-done", KNotification::CloseOnTimeout);
                notification->setText(i18nc("indicator_name, removed success","<b>%0</b> indicator removed successfully").arg(name));
                notification->sendEvent();
            };

            if (result == QMessageBox::Yes) {
                qDebug() << "Trying to remove indicator :: " << id;
                QProcess process;
                process.start(QString("kpackagetool5 -r " +id + " -t Latte/Indicator"));
                process.waitForFinished();
                showRemovedSucceed(pluginName);
            }
        });

        msg->open();
    }
}

void Factory::downloadIndicator()
{
    KNS3::DownloadDialog dialog(QStringLiteral("latte-indicators.knsrc"), m_parentWidget);

    dialog.exec();
}

}
}
