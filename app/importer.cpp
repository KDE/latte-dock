/*
*  Copyright 2017  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "importer.h"

#include "layoutmanager.h"
#include "layoutsettings.h"
#include "screenpool.h"
#include "../liblattedock/dock.h"

#include <QFile>
#include <QTemporaryDir>

#include <KLocalizedString>
#include <KArchive/KTar>

namespace Latte {

Importer::Importer(QObject *parent)
    : QObject(parent)
{
    m_manager = qobject_cast<LayoutManager *>(parent);
}

Importer::~Importer()
{
}

bool Importer::updateOldConfiguration()
{
    QFile oldAppletsFile(QDir::homePath() + "/.config/lattedock-appletsrc");

    if (!oldAppletsFile.exists()) {
        return false;
    }

    //! import standard old configuration and create the relevant layouts
    importOldLayout(QDir::homePath() + "/.config/lattedock-appletsrc", i18n("My Layout"));
    importOldLayout(QDir::homePath() + "/.config/lattedock-appletsrc", i18n("Alternative"), true);

    //! global settings that must be imported to universal
    KSharedConfigPtr oldFile = KSharedConfig::openConfig(QDir::homePath() + "/.config/lattedock-appletsrc");
    KConfigGroup oldGeneralSettings = KConfigGroup(oldFile, "General");
    bool exposeLayoutsMenu = oldGeneralSettings.readEntry("exposeAltSession", false);

    if (m_manager) {
        m_manager->corona()->universalSettings()->setExposeLayoutsMenu(exposeLayoutsMenu);
    }


    QFile extFile(QDir::homePath() + "/.config/lattedockextrc");

    //! import also the old user layouts into the new architecture
    if (extFile.exists()) {
        KSharedConfigPtr extFileConfig = KSharedConfig::openConfig(extFile.fileName());
        KConfigGroup externalSettings = KConfigGroup(extFileConfig, "External");
        QStringList userLayouts = externalSettings.readEntry("userLayouts", QStringList());

        foreach (auto userConfig, userLayouts) {
            qDebug() << "user layout : " << userConfig;
            importOldConfiguration(userConfig);
        }
    }

    return true;
}

bool Importer::importOldLayout(QString oldAppletsPath, QString newName, bool alternative)
{
    QString newLayoutPath = layoutCanBeImported(oldAppletsPath, newName);
    qDebug() << "New Layout Should be created: " << newLayoutPath;

    KSharedConfigPtr oldFile = KSharedConfig::openConfig(oldAppletsPath);
    KSharedConfigPtr newFile = KSharedConfig::openConfig(newLayoutPath);

    KConfigGroup containments = KConfigGroup(oldFile, "Containments");
    KConfigGroup copiedContainments = KConfigGroup(newFile, "Containments");

    QList<int> systrays;

    bool atLeastOneContainmentWasFound{false};

    //! first copy the latte containments that correspond to the correct session
    //! and find also the systrays that should be copied also
    foreach (auto containmentId, containments.groupList()) {
        KConfigGroup containmentGroup = containments.group(containmentId);

        QString plugin = containmentGroup.readEntry("plugin", QString());
        Dock::SessionType session = (Dock::SessionType)containmentGroup.readEntry("session", (int)Dock::DefaultSession);

        bool shouldImport = false;

        if (plugin == "org.kde.latte.containment" && session == Dock::DefaultSession && !alternative) {
            qDebug() << containmentId << " - " << plugin << " - " << session;
            shouldImport = true;
        } else if (plugin == "org.kde.latte.containment" && session == Dock::AlternativeSession && alternative) {
            qDebug() << containmentId << " - " << plugin << " - " << session;
            shouldImport = true;
        }

        // this latte containment should be imported
        if (shouldImport) {
            auto applets = containments.group(containmentId).group("Applets");

            foreach (auto applet, applets.groupList()) {
                KConfigGroup appletSettings = applets.group(applet).group("Configuration");

                int systrayId = appletSettings.readEntry("SystrayContainmentId", "-1").toInt();

                if (systrayId != -1) {
                    systrays.append(systrayId);
                    qDebug() << "systray was found in the containment...";
                    break;
                }
            }

            KConfigGroup newContainment = copiedContainments.group(containmentId);
            containmentGroup.copyTo(&newContainment);
            atLeastOneContainmentWasFound = true;
        }
    }

    //! not even one latte containment was found for that layout so we must break
    //! the code here
    if (!atLeastOneContainmentWasFound) {
        return false;
    }

    //! copy also the systrays that were discovered
    foreach (auto containmentId, containments.groupList()) {
        int cId = containmentId.toInt();

        if (systrays.contains(cId)) {
            KConfigGroup containmentGroup = containments.group(containmentId);
            KConfigGroup newContainment = copiedContainments.group(containmentId);
            containmentGroup.copyTo(&newContainment);
        }
    }

    copiedContainments.sync();

    KConfigGroup oldGeneralSettings = KConfigGroup(oldFile, "General");

    bool syncLaunchers{false};
    QStringList globalLaunchers;

    if (!alternative) {
        syncLaunchers = oldGeneralSettings.readEntry("syncLaunchers_default", false);
        globalLaunchers = oldGeneralSettings.readEntry("globalLaunchers_default", QStringList());
    } else {
        syncLaunchers = oldGeneralSettings.readEntry("syncLaunchers_alternative", false);
        globalLaunchers = oldGeneralSettings.readEntry("globalLaunchers_alternative", QStringList());
    }

    //! update also the layout settings correctly
    LayoutSettings newLayout(this, newLayoutPath);
    newLayout.setVersion(2);
    newLayout.setSyncLaunchers(syncLaunchers);
    newLayout.setGlobalLaunchers(globalLaunchers);

    return true;
}

QString Importer::layoutCanBeImported(QString oldAppletsPath, QString newName)
{
    QFile oldAppletsrc(oldAppletsPath);

    //! old file doesnt exist
    if (!oldAppletsrc.exists()) {
        return QString();
    }

    LayoutSettings oldLSettings(this, oldAppletsPath);

    //! old file layout appears to not be old as its version is >=2
    if (oldLSettings.version() >= 2) {
        return QString();
    }

    QDir layoutDir(QDir::homePath() + "/.config/latte");

    if (!layoutDir.exists()) {
        QDir(QDir::homePath() + "/.config").mkdir("latte");
    }

    //! set up the new layout name
    if (newName.isEmpty()) {
        int extension = oldAppletsrc.fileName().lastIndexOf(".latterc");

        if (extension > 0) {
            //! remove the last 8 characters that contain the extension
            newName = oldAppletsrc.fileName().remove(extension, 8);
        } else {
            newName = oldAppletsrc.fileName();
        }
    }

    QFile newLayoutFile(layoutDir.absolutePath() + "/" + newName + ".layout.latte");
    QString newLayoutPath = newLayoutFile.fileName();

    QStringList filter;
    filter.append(QString(newName + "*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    //! if the newLayout already exists provide a newName that doesnt
    if (files.count() >= 1) {
        int newCounter = files.count() + 1;

        newLayoutPath = layoutDir.absolutePath() + "/" + newName + "-" + QString::number(newCounter) + ".layout.latte";
    }

    return newLayoutPath;
}

bool Importer::importOldConfiguration(QString oldConfigPath, QString newName)
{
    QFile oldConfigFile(oldConfigPath);

    if (!oldConfigFile.exists()) {
        return false;
    }

    KTar archive(oldConfigPath, QStringLiteral("application/x-tar"));
    archive.open(QIODevice::ReadOnly);

    if (!archive.isOpen()) {
        return false;
    }

    auto rootDir = archive.directory();
    QTemporaryDir uniqueTempDir;
    QDir tempDir{uniqueTempDir.path()};

    qDebug() << "temp layout directory : " << tempDir.absolutePath();

    if (rootDir) {
        if (!tempDir.exists())
            tempDir.mkpath(tempDir.absolutePath());

        foreach (auto &name, rootDir->entries()) {
            auto fileEntry = rootDir->file(name);

            if (fileEntry && (fileEntry->name() == "lattedockrc"
                              || fileEntry->name() == "lattedock-appletsrc")) {
                if (!fileEntry->copyTo(tempDir.absolutePath())) {
                    qInfo() << i18nc("import/export config", "The extracted file coulnd be copied!!!");
                    archive.close();
                    return false;
                }
            } else {
                qInfo() << i18nc("import/export config", "The file has a wrong format!!!");
                archive.close();
                return false;
            }
        }
    } else {
        qInfo() << i18nc("import/export config", "The temp directory couldnt be created!!!");
        archive.close();
        return false;
    }

    //! only if the above has passed we must process the files
    QString appletsPath(tempDir.absolutePath() + "/lattedock-appletsrc");
    QString screensPath(tempDir.absolutePath() + "/lattedockrc");

    if (!QFile(appletsPath).exists() || !QFile(screensPath).exists()) {
        return false;
    }


    if (newName.isEmpty()) {
        int lastSlash = oldConfigPath.lastIndexOf("/");
        newName = oldConfigPath.remove(0, lastSlash + 1);

        int ext = newName.lastIndexOf(".latterc");
        newName = newName.remove(ext, 8);
    }

    if (!importOldLayout(appletsPath, newName)) {
        return false;
    }

    //! the old configuration contains also screen values, these must be updated also
    KSharedConfigPtr oldScreensConfig = KSharedConfig::openConfig(screensPath);
    KConfigGroup m_screensGroup = KConfigGroup(oldScreensConfig, "ScreenConnectors");

    //restore the known ids to connector mappings
    foreach (const QString &key, m_screensGroup.keyList()) {
        QString connector = m_screensGroup.readEntry(key, QString());
        int id = key.toInt();

        if (id >= 10 && !m_manager->corona()->screenPool()->knownIds().contains(id)) {
            m_manager->corona()->screenPool()->insertScreenMapping(id, connector);
        }
    }

    return true;
}

}
