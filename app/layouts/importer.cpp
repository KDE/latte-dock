/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2017 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "importer.h"

// local
#include <coretypes.h>
#include "manager.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layout/abstractlayout.h"
#include "../settings/universalsettings.h"
#include "../templates/templatesmanager.h"
#include "../tools/commontools.h"

// Qt
#include <QFile>
#include <QLatin1String>

// KDE
#include <KArchive/KTar>
#include <KArchive/KArchiveEntry>
#include <KArchive/KArchiveDirectory>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KNotification>


enum SessionType
{
    DefaultSession = 0,
    AlternativeSession
};

namespace Latte {
namespace Layouts {

Importer::Importer(QObject *parent)
    : QObject(parent)
{
    m_manager = qobject_cast<Layouts::Manager *>(parent);

    qDebug() << " IMPORTER, STORAGE TEMP DIR ::: " << m_storageTmpDir.path();
}

Importer::~Importer()
{
}

bool Importer::updateOldConfiguration()
{
    QFile oldAppletsFile(Latte::configPath() + "/lattedock-appletsrc");

    if (!oldAppletsFile.exists()) {
        return false;
    }

    //! import standard old configuration and create the relevant layouts
    importOldLayout(Latte::configPath() + "/lattedock-appletsrc", i18n("My Layout"));
    importOldLayout(Latte::configPath() + "/lattedock-appletsrc", i18n("Alternative"), true);

    QFile extFile(Latte::configPath() + "/lattedockextrc");

    //! import also the old user layouts into the new architecture
    if (extFile.exists()) {
        KSharedConfigPtr extFileConfig = KSharedConfig::openConfig(extFile.fileName());
        KConfigGroup externalSettings = KConfigGroup(extFileConfig, "External");
        QStringList userLayouts = externalSettings.readEntry("userLayouts", QStringList());

        for(const auto &userConfig : userLayouts) {
            qDebug() << "user layout : " << userConfig;
            importOldConfiguration(userConfig);
        }
    }

    m_manager->corona()->universalSettings()->setVersion(2);
    m_manager->corona()->universalSettings()->setSingleModeLayoutName(i18n("My Layout"));

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
    for(const auto &containmentId : containments.groupList()) {
        KConfigGroup containmentGroup = containments.group(containmentId);

        QString plugin = containmentGroup.readEntry("plugin", QString());
        SessionType session = (SessionType)containmentGroup.readEntry("session", (int)DefaultSession);

        bool shouldImport = false;

        if (plugin == QLatin1String("org.kde.latte.containment") && session == DefaultSession && !alternative) {
            qDebug() << containmentId << " - " << plugin << " - " << session;
            shouldImport = true;
        } else if (plugin == QLatin1String("org.kde.latte.containment") && session == AlternativeSession && alternative) {
            qDebug() << containmentId << " - " << plugin << " - " << session;
            shouldImport = true;
        }

        // this latte containment should be imported
        if (shouldImport) {
            auto applets = containments.group(containmentId).group("Applets");

            for(const auto &applet : applets.groupList()) {
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
    for(const auto &containmentId : containments.groupList()) {
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
    Layout::AbstractLayout newLayout(this, newLayoutPath, newName);
    newLayout.setVersion(2);
    newLayout.setLaunchers(layoutLaunchers);

    //newLayout.setShowInMenu(true);

    if (alternative) {
        newLayout.setColor("purple");
    } else {
        newLayout.setColor("blue");
    }

    return true;
}

QStringList Importer::standardPaths(bool localfirst)
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    if (localfirst) {
        return paths;
    } else {
        QStringList reversed;

        for (int i=paths.count()-1; i>=0; i--) {
            reversed << paths[i];
        }

        return reversed;
    }
}

QStringList Importer::standardPathsFor(QString subPath, bool localfirst)
{
    QStringList paths = standardPaths(localfirst);

    QString separator = subPath.startsWith("/") ? "" : "/";

    for (int i=0; i<paths.count(); ++i) {
        paths[i] = paths[i] + separator + subPath;
    }

    return paths;
}

QString Importer::standardPath(QString subPath, bool localfirst)
{
    QStringList paths = standardPaths(localfirst);

    for(const auto &pt : paths) {
        QString ptF = pt + "/" +subPath;
        if (QFileInfo(ptF).exists()) {
            return ptF;
        }
    }

    //! in any case that above fails
    if (QFileInfo("/usr/share/"+subPath).exists()) {
        return "/usr/share/"+subPath;
    }

    return "";
}

QString Importer::storageTmpDir() const
{
    return m_storageTmpDir.path();
}

QString Importer::layoutCanBeImported(QString oldAppletsPath, QString newName, QString exportDirectory)
{
    QFile oldAppletsrc(oldAppletsPath);

    //! old file doesn't exist
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

    QDir layoutDir(exportDirectory.isNull() ? layoutUserDir() : exportDirectory);

    if (!layoutDir.exists() && exportDirectory.isNull()) {
        QDir(Latte::configPath()).mkdir("latte");
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

    //! if the newLayout already exists provide a newName that doesn't
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

        for(const auto &name : rootDir->entries()) {
            auto fileEntry = rootDir->file(name);

            if (fileEntry && (fileEntry->name() == QLatin1String("lattedockrc")
                              || fileEntry->name() == QLatin1String("lattedock-appletsrc"))) {
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
    /*
    * do not use any deprecated screen ids
    * KSharedConfigPtr oldScreensConfig = KSharedConfig::openConfig(screensPath);
    KConfigGroup m_screensGroup = KConfigGroup(oldScreensConfig, "ScreenConnectors");

    //restore the known ids to connector mappings
    for(const QString &key : m_screensGroup.keyList()) {
        QString connector = m_screensGroup.readEntry(key, QString());
        int id = key.toInt();

        if (id >= 10 && !m_manager->corona()->screenPool()->hasScreenId(id)) {
            m_manager->corona()->screenPool()->insertScreenMapping(id, connector);
        }
    }*/

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

    archive.addLocalFile(QString(Latte::configPath() + "/lattedockrc"), QStringLiteral("lattedockrc"));

    for(const auto &layoutName : availableLayouts()) {
        archive.addLocalFile(layoutUserFilePath(layoutName), QString("latte/" + layoutName + ".layout.latte"));
    }

    //! custom templates
    QDir templatesDir(Latte::configPath() + "/latte/templates");
    QStringList filters;
    filters.append(QString("*.layout.latte"));
    QStringList templates = templatesDir.entryList(filters, QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    for (int i=0; i<templates.count(); ++i) {
        QString templatePath = templatesDir.path() + "/" + templates[i];
        archive.addLocalFile(templatePath, QString("latte/templates/" + templates[i]));
    }

    filters.clear();
    filters.append(QString("*.view.latte"));
    templates = templatesDir.entryList(filters, QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    for (int i=0; i<templates.count(); ++i) {
        QString templatePath = templatesDir.path() + "/" + templates[i];
        archive.addLocalFile(templatePath, QString("latte/templates/" + templates[i]));
    }

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

    if (version1applets) {
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

    QDir latteDir(layoutUserDir());

    if (latteDir.exists()) {
        latteDir.removeRecursively();
    }

    archive.directory()->copyTo(Latte::configPath());

    return true;
}

bool Importer::isAutostartEnabled()
{
    QFile autostartFile(Latte::configPath() + "/autostart/org.kde.latte-dock.desktop");
    return autostartFile.exists();
}

void Importer::enableAutostart()
{
    //! deprecated old file
    QFile oldAutostartFile(Latte::configPath() + "/autostart/latte-dock.desktop");

    if (oldAutostartFile.exists()) {
        //! remove deprecated file
        oldAutostartFile.remove();
    }

    QFile autostartFile(Latte::configPath() + "/autostart/org.kde.latte-dock.desktop");
    QFile metaFile(standardPath("applications/org.kde.latte-dock.desktop", false));

    if (autostartFile.exists()) {
        //! if autostart file already exists, do nothing
        return;
    }

    if (metaFile.exists()) {
        //! check if autostart folder exists and create otherwise
        QDir autostartDir(Latte::configPath() + "/autostart");
        if (!autostartDir.exists()) {
            QDir configDir(Latte::configPath());
            configDir.mkdir("autostart");
        }

        metaFile.copy(autostartFile.fileName());
    }
}

void Importer::disableAutostart()
{
    QFile oldAutostartFile(Latte::configPath() + "/autostart/latte-dock.desktop");

    if (oldAutostartFile.exists()) {
        //! remove deprecated file
        oldAutostartFile.remove();
    }

    QFile autostartFile(Latte::configPath() + "/autostart/org.kde.latte-dock.desktop");

    if (autostartFile.exists()) {
        autostartFile.remove();
    }
}

bool Importer::hasViewTemplate(const QString &name)
{
    return availableViewTemplates().contains(name);
}

QString Importer::importLayout(const QString &fileName, const QString &suggestedLayoutName)
{
    QString newLayoutName = importLayoutHelper(fileName, suggestedLayoutName);

    if (!newLayoutName.isEmpty()) {
        emit newLayoutAdded(layoutUserFilePath(newLayoutName));
    }

    return newLayoutName;
}

QString Importer::importLayoutHelper(const QString &fileName, const QString &suggestedLayoutName)
{
    LatteFileVersion version = fileVersion(fileName);

    if (version != LayoutVersion2) {
        return QString();
    }

    QString newLayoutName = !suggestedLayoutName.isEmpty() ? suggestedLayoutName : Layout::AbstractLayout::layoutName(fileName);
    newLayoutName = uniqueLayoutName(newLayoutName);

    QString newPath = layoutUserFilePath(newLayoutName);

    QDir localLayoutsDir(layoutUserDir());

    if (!localLayoutsDir.exists()) {
        QDir(Latte::configPath()).mkdir("latte");
    }

    QFile(fileName).copy(newPath);

    QFileInfo newFileInfo(newPath);

    if (newFileInfo.exists() && !newFileInfo.isWritable()) {
        QFile(newPath).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }

    return newLayoutName;
}

QStringList Importer::availableLayouts()
{
    QDir layoutDir(layoutUserDir());
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    QStringList layoutNames;

    for(const auto &file : files) {
        layoutNames.append(Layout::AbstractLayout::layoutName(file));
    }

    return layoutNames;
}

QStringList Importer::availableViewTemplates()
{
    QStringList templates;

    QDir localDir(layoutUserDir() + "/templates");
    QStringList filter;
    filter.append(QString("*.view.latte"));
    QStringList files = localDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    for(const auto &file : files) {
        templates.append(Templates::Manager::templateName(file));
    }

    QDir systemDir(systemShellDataPath()+"/contents/templates");
    QStringList sfiles = systemDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    for(const auto &file : sfiles) {
        QString name = Templates::Manager::templateName(file);
        if (!templates.contains(name)) {
            templates.append(name);
        }
    }

    return templates;
}

QStringList Importer::availableLayoutTemplates()
{
    QStringList templates;

    QDir localDir(layoutUserDir() + "/templates");
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList files = localDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    for(const auto &file : files) {
        templates.append(Templates::Manager::templateName(file));
    }

    QDir systemDir(systemShellDataPath()+"/contents/templates");
    QStringList sfiles = systemDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    for(const auto &file : sfiles) {
        QString name = Templates::Manager::templateName(file);
        if (!templates.contains(name)) {
            templates.append(name);
        }
    }

    return templates;
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
    return QFile::exists(layoutUserFilePath(layoutName));
}


QString Importer::layoutUserDir()
{
    return QString(Latte::configPath() + "/latte");
}

QString Importer::layoutUserFilePath(QString layoutName)
{
    return QString(layoutUserDir() + "/" + layoutName + ".layout.latte");
}

QString Importer::systemShellDataPath()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    QString rootpath = paths.count() > 0 ? paths[paths.count()-1] : "/usr/share";
    return  rootpath + "/plasma/shells/org.kde.latte.shell";
}

QString Importer::layoutTemplateSystemFilePath(const QString &name)
{
    return systemShellDataPath() + "/contents/templates/" + name + ".layout.latte";
}

QString Importer::uniqueLayoutName(QString name)
{
    int pos_ = name.lastIndexOf(QRegExp(QString(" - [0-9]+")));

    if (layoutExists(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (layoutExists(name)) {
        name = namePart + " - " + QString::number(i);
        i++;
    }

    return name;
}

Latte::MultipleLayouts::Status Importer::multipleLayoutsStatus()
{
    QString linkedFilePath = layoutUserFilePath(Layout::MULTIPLELAYOUTSHIDDENNAME);
    if (!QFileInfo(linkedFilePath).exists()) {
        return Latte::MultipleLayouts::Uninitialized;
    }

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(linkedFilePath);
    KConfigGroup multipleSettings = KConfigGroup(filePtr, "MultipleLayoutsSettings");
    return static_cast<Latte::MultipleLayouts::Status>(multipleSettings.readEntry("status", (int)Latte::MultipleLayouts::Uninitialized));
}

void Importer::setMultipleLayoutsStatus(const Latte::MultipleLayouts::Status &status)
{
    QString linkedFilePath = layoutUserFilePath(Layout::MULTIPLELAYOUTSHIDDENNAME);

    if (!QFileInfo(linkedFilePath).exists()) {
        return;
    }

    if (multipleLayoutsStatus() == status) {
        return;
    }

    qDebug() << " MULTIPLE LAYOUTS changed status:" << status;

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(linkedFilePath);
    KConfigGroup multipleSettings = KConfigGroup(filePtr, "MultipleLayoutsSettings");
    multipleSettings.writeEntry("status", (int)(status));
    multipleSettings.sync();
}

QStringList Importer::checkRepairMultipleLayoutsLinkedFile()
{
    QString linkedFilePath = layoutUserFilePath(Layout::MULTIPLELAYOUTSHIDDENNAME);
    KSharedConfigPtr filePtr = KSharedConfig::openConfig(linkedFilePath);
    KConfigGroup linkedContainments = KConfigGroup(filePtr, "Containments");

    //! layoutName and its Containments
    QHash<QString, QStringList> linkedLayoutContainmentGroups;

    for(const auto &cId : linkedContainments.groupList()) {
        QString layoutName = linkedContainments.group(cId).readEntry("layoutId", QString());

        if (!layoutName.isEmpty()) {
            qDebug() << layoutName;
            linkedLayoutContainmentGroups[layoutName].append(cId);
            linkedContainments.group(cId).writeEntry("layoutId", QString());
        }
    }

    QStringList updatedLayouts;

    for(const auto &layoutName : linkedLayoutContainmentGroups.uniqueKeys()) {
        if (layoutName != Layout::MULTIPLELAYOUTSHIDDENNAME && layoutExists(layoutName)) {
            updatedLayouts << layoutName;
            KSharedConfigPtr layoutFilePtr = KSharedConfig::openConfig(layoutUserFilePath(layoutName));
            KConfigGroup origLayoutContainments = KConfigGroup(layoutFilePtr, "Containments");

            //Clear old containments
            origLayoutContainments.deleteGroup();

            //Update containments
            for(const auto &cId : linkedLayoutContainmentGroups[layoutName]) {
                KConfigGroup newContainment = origLayoutContainments.group(cId);
                linkedContainments.group(cId).copyTo(&newContainment);
                linkedContainments.group(cId).deleteGroup();
            }

            origLayoutContainments.sync();
        }
    }

    //! clear all remaining ghost containments
    for(const auto &cId : linkedContainments.groupList()) {
        linkedContainments.group(cId).deleteGroup();
    }

    linkedContainments.sync();

    return updatedLayouts;
}

}
}
