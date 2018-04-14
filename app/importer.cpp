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

#include "dockcorona.h"
#include "layoutmanager.h"
#include "layout.h"
#include "screenpool.h"
#include "universalsettings.h"
#include "../liblattedock/dock.h"

#include <QFile>
#include <QTemporaryDir>

#include <KArchive/KTar>
#include <KArchive/KArchiveEntry>
#include <KArchive/KArchiveDirectory>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KNotification>


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

    m_manager->corona()->universalSettings()->setCurrentLayoutName(i18n("My Layout"));
    m_manager->corona()->universalSettings()->setVersion(2);

    return true;
}

bool Importer::importOldLayout(QString oldAppletsPath, QString newName, bool alternative, QString exportDirectory)
{
    QString newLayoutPath = layoutCanBeImported(oldAppletsPath, newName, exportDirectory);
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

    QStringList layoutLaunchers;

    if (!alternative) {
        layoutLaunchers = oldGeneralSettings.readEntry("globalLaunchers_default", QStringList());
    } else {
        layoutLaunchers = oldGeneralSettings.readEntry("globalLaunchers_alternative", QStringList());
    }

    //! update also the layout settings correctly
    Layout newLayout(this, newLayoutPath, newName);
    newLayout.setVersion(2);
    newLayout.setLaunchers(layoutLaunchers);

    newLayout.setShowInMenu(true);

    if (alternative) {
        newLayout.setColor("purple");
    } else {
        newLayout.setColor("blue");
    }

    return true;
}

QString Importer::layoutCanBeImported(QString oldAppletsPath, QString newName, QString exportDirectory)
{
    QFile oldAppletsrc(oldAppletsPath);

    //! old file doesnt exist
    if (!oldAppletsrc.exists()) {
        return QString();
    }

    KSharedConfigPtr lConfig = KSharedConfig::openConfig(oldAppletsPath);
    KConfigGroup m_layoutGroup = KConfigGroup(lConfig, "LayoutSettings");
    int layoutVersion = m_layoutGroup.readEntry("version", 1);

    //! old file layout appears to not be old as its version is >=2
    if (layoutVersion >= 2) {
        return QString();
    }

    QDir layoutDir(exportDirectory.isNull() ? QDir::homePath() + "/.config/latte" : exportDirectory);

    if (!layoutDir.exists() && exportDirectory.isNull()) {
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

    QString newLayoutPath = layoutDir.absolutePath() + "/" + newName + ".layout.latte";
    QFile newLayoutFile(newLayoutPath);

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
                    qInfo() << i18nc("import/export config", "The extracted file could not be copied!!!");
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
        qInfo() << i18nc("import/export config", "The temp directory could not be created!!!");
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

bool Importer::exportFullConfiguration(QString file)
{
    if (QFile::exists(file) && !QFile::remove(file)) {
        return false;
    }

    KTar archive(file, QStringLiteral("application/x-tar"));

    if (!archive.open(QIODevice::WriteOnly)) {
        return false;
    }

    archive.addLocalFile(QString(QDir::homePath() + "/.config/lattedockrc"), QStringLiteral("lattedockrc"));

    foreach (auto layoutName, availableLayouts()) {
        archive.addLocalFile(layoutFilePath(layoutName), QString("latte/" + layoutName + ".layout.latte"));
    }

    //archive.addLocalDirectory(QString(QDir::homePath() + "/.config/latte"), QStringLiteral("latte"));

    archive.close();

    return true;
}

Importer::LatteFileVersion Importer::fileVersion(QString file)
{
    if (!QFile::exists(file))
        return UnknownFileType;

    if (file.endsWith(".layout.latte")) {
        KSharedConfigPtr lConfig = KSharedConfig::openConfig(QFileInfo(file).absoluteFilePath());
        KConfigGroup layoutGroup = KConfigGroup(lConfig, "LayoutSettings");
        int version = layoutGroup.readEntry("version", 1);

        if (version == 2)
            return Importer::LayoutVersion2;
        else
            return Importer::UnknownFileType;
    }

    if (!file.endsWith(".latterc")) {
        return Importer::UnknownFileType;
    }

    KTar archive(file, QStringLiteral("application/x-tar"));
    archive.open(QIODevice::ReadOnly);

    //! if the file isnt a tar archive
    if (!archive.isOpen()) {
        return Importer::UnknownFileType;
    }

    QTemporaryDir archiveTempDir;

    bool version1rc = false;
    bool version1applets = false;

    bool version2rc = false;
    bool version2LatteDir = false;
    bool version2layout = false;

    archive.directory()->copyTo(archiveTempDir.path());


    //rc file
    QString rcFile(archiveTempDir.path() + "/lattedockrc");

    if (QFile(rcFile).exists()) {
        KSharedConfigPtr lConfig = KSharedConfig::openConfig(rcFile);
        KConfigGroup universalGroup = KConfigGroup(lConfig, "UniversalSettings");
        int version = universalGroup.readEntry("version", 1);

        if (version == 1) {
            version1rc = true;
        } else if (version == 2) {
            version2rc = true;
        }
    }

    //applets file
    QString appletsFile(archiveTempDir.path() + "/lattedock-appletsrc");

    if (QFile(appletsFile).exists() && version1rc) {
        KSharedConfigPtr lConfig = KSharedConfig::openConfig(appletsFile);
        KConfigGroup generalGroup = KConfigGroup(lConfig, "LayoutSettings");
        int version = generalGroup.readEntry("version", 1);

        if (version == 1) {
            version1applets = true;
        } else if (version == 2) {
            version2layout = true;
        }
    }

    //latte directory
    QString latteDir(archiveTempDir.path() + "/latte");

    if (QDir(latteDir).exists()) {
        version2LatteDir = true;
    }

    if (version1applets && version1applets) {
        return ConfigVersion1;
    } else if (version2rc && version2LatteDir) {
        return ConfigVersion2;
    }

    return Importer::UnknownFileType;
}

bool Importer::importHelper(QString fileName)
{
    LatteFileVersion version = fileVersion(fileName);

    if ((version != ConfigVersion1) && (version != ConfigVersion2)) {
        return false;
    }

    KTar archive(fileName, QStringLiteral("application/x-tar"));
    archive.open(QIODevice::ReadOnly);

    if (!archive.isOpen()) {
        return false;
    }

    QString latteDirPath(QDir::homePath() + "/.config/latte");
    QDir latteDir(latteDirPath);

    if (latteDir.exists()) {
        latteDir.removeRecursively();
    }

    archive.directory()->copyTo(QString(QDir::homePath() + "/.config"));

    return true;
}

QString Importer::importLayoutHelper(QString fileName)
{
    LatteFileVersion version = fileVersion(fileName);

    if (version != LayoutVersion2) {
        return QString();
    }

    QString newLayoutName = Layout::layoutName(fileName);
    newLayoutName = uniqueLayoutName(newLayoutName);

    QString newPath = QDir::homePath() + "/.config/latte/" + newLayoutName + ".layout.latte";
    QFile(fileName).copy(newPath);

    QFileInfo newFileInfo(newPath);

    if (newFileInfo.exists() && !newFileInfo.isWritable()) {
        QFile(newPath).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }

    return newLayoutName;

}

QStringList Importer::availableLayouts()
{
    QDir layoutDir(QDir::homePath() + "/.config/latte");
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    QStringList layoutNames;

    foreach (auto file, files) {
        layoutNames.append(Layout::layoutName(file));
    }

    return layoutNames;
}


QString Importer::nameOfConfigFile(const QString &fileName)
{
    int lastSlash = fileName.lastIndexOf("/");
    QString tempLayoutFile = fileName;
    QString layoutName = tempLayoutFile.remove(0, lastSlash + 1);

    int ext = layoutName.lastIndexOf(".latterc");
    layoutName = layoutName.remove(ext, 8);

    return layoutName;
}

bool Importer::layoutExists(QString layoutName)
{
    return QFile::exists(layoutFilePath(layoutName));
}


QString Importer::layoutFilePath(QString layoutName)
{
    return QString(QDir::homePath() + "/.config/latte/" + layoutName + ".layout.latte");
}

QString Importer::uniqueLayoutName(QString name)
{
    int pos_ = name.lastIndexOf(QRegExp(QString("[-][0-9]+")));

    if (layoutExists(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (layoutExists(name)) {
        name = namePart + "-" + QString::number(i);
        i++;
    }

    return name;
}

QStringList Importer::checkRepairMultipleLayoutsLinkedFile()
{
    QString linkedFilePath = QDir::homePath() + "/.config/latte/" + Layout::MultipleLayoutsName + ".layout.latte";
    KSharedConfigPtr filePtr = KSharedConfig::openConfig(linkedFilePath);
    KConfigGroup linkedContainments = KConfigGroup(filePtr, "Containments");

    //! layoutName and its Containments
    QHash<QString, QStringList> linkedLayoutContainmentGroups;

    foreach (auto cId, linkedContainments.groupList()) {
        QString layoutName = linkedContainments.group(cId).readEntry("layoutId", QString());

        if (!layoutName.isEmpty()) {
            qDebug() << layoutName;
            linkedLayoutContainmentGroups[layoutName].append(cId);
            linkedContainments.group(cId).writeEntry("layoutId", QString());
        }
    }

    QStringList updatedLayouts;

    foreach (auto layoutName, linkedLayoutContainmentGroups.uniqueKeys()) {
        if (layoutName != Layout::MultipleLayoutsName && layoutExists(layoutName)) {
            updatedLayouts << layoutName;
            KSharedConfigPtr layoutFilePtr = KSharedConfig::openConfig(layoutFilePath(layoutName));
            KConfigGroup origLayoutContainments = KConfigGroup(layoutFilePtr, "Containments");

            //Clear old containments
            origLayoutContainments.deleteGroup();

            //Update containments
            foreach (auto cId, linkedLayoutContainmentGroups[layoutName]) {
                KConfigGroup newContainment = origLayoutContainments.group(cId);
                linkedContainments.group(cId).copyTo(&newContainment);
                linkedContainments.group(cId).deleteGroup();
            }

            origLayoutContainments.sync();
        }
    }

    //! clear all remaining ghost containments
    foreach (auto cId, linkedContainments.groupList()) {
        linkedContainments.group(cId).deleteGroup();
    }

    linkedContainments.sync();

    return updatedLayouts;
}

}
