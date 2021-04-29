/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "storage.h"

// local
#include <coretypes.h>
#include "importer.h"
#include "manager.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../data/errordata.h"
#include "../layout/abstractlayout.h"
#include "../view/view.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

// KDE
#include <KConfigGroup>
#include <KPluginMetaData>
#include <KSharedConfig>
#include <KPackage/Package>
#include <KPackage/PackageLoader>

// Plasma
#include <Plasma>
#include <Plasma/Applet>
#include <Plasma/Containment>

namespace Latte {
namespace Layouts {

const int Storage::IDNULL = -1;
const int Storage::IDBASE = 0;

Storage::Storage()
{
    qDebug() << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> LAYOUTS::STORAGE, TEMP DIR ::: " << m_storageTmpDir.path();

    //! Known Errors
    m_knownErrors << Data::Generic(Data::Error::APPLETSWITHSAMEID, i18n("Different Applets With Same Id"));
    m_knownErrors << Data::Generic(Data::Error::APPLETSANDCONTAINMENTSWITHSAMEID, i18n("Different Applets And Containments With Same Id"));
    m_knownErrors << Data::Generic(Data::Error::ORPHANEDSUBCONTAINMENTS, i18n("Orphaned Subcontainments"));

    SubContaimentIdentityData data;

    //! Systray
    m_subIdentities << SubContaimentIdentityData{.cfgGroup="Configuration", .cfgProperty="SystrayContainmentId"};
    //! Group applet
    m_subIdentities << SubContaimentIdentityData{.cfgGroup="Configuration", .cfgProperty="ContainmentId"};
}

Storage::~Storage()
{
}

Storage *Storage::self()
{
    static Storage store;
    return &store;
}

bool Storage::isWritable(const Layout::GenericLayout *layout) const
{
    QFileInfo layoutFileInfo(layout->file());

    if (layoutFileInfo.exists() && !layoutFileInfo.isWritable()) {
        return false;
    } else {
        return true;
    }
}

bool Storage::isLatteContainment(const Plasma::Containment *containment) const
{
    if (!containment) {
        return false;
    }

    if (containment->pluginMetaData().pluginId() == "org.kde.latte.containment") {
        return true;
    }

    return false;
}

bool Storage::isLatteContainment(const KConfigGroup &group) const
{
    QString pluginId = group.readEntry("plugin", "");
    return pluginId == "org.kde.latte.containment";
}

bool Storage::isSubContainment(const Layout::GenericLayout *layout, const Plasma::Applet *applet) const
{
    if (!layout || !applet) {
        return false;
    }

    for (const auto containment : *layout->containments()) {
        Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(containment->parent());
        if (parentApplet && parentApplet == applet) {
            return true;
        }
    }

    return false;
}

bool Storage::isSubContainment(const KConfigGroup &appletGroup) const
{
    return isValid(subContainmentId(appletGroup));
}

bool Storage::isValid(const int &id)
{
    return id >= IDBASE;
}

int Storage::subContainmentId(const KConfigGroup &appletGroup) const
{
    //! cycle through subcontainments identities
    for (auto subidentity : m_subIdentities) {
        KConfigGroup appletConfigGroup = appletGroup;

        if (!subidentity.cfgGroup.isEmpty()) {
            //! if identity provides specific configuration group
            if (appletConfigGroup.hasGroup(subidentity.cfgGroup)) {
                appletConfigGroup = appletGroup.group(subidentity.cfgGroup);
            }
        }

        if (!subidentity.cfgProperty.isEmpty()) {
            //! if identity provides specific property for configuration group
            if (appletConfigGroup.hasKey(subidentity.cfgProperty)) {
                return appletConfigGroup.readEntry(subidentity.cfgProperty, IDNULL);
            }
        }
    }

    return IDNULL;
}

int Storage::subIdentityIndex(const KConfigGroup &appletGroup) const
{
    if (!isSubContainment(appletGroup)) {
        return IDNULL;
    }

    //! cycle through subcontainments identities
    for (int i=0; i<m_subIdentities.count(); ++i) {
        KConfigGroup appletConfigGroup = appletGroup;

        if (!m_subIdentities[i].cfgGroup.isEmpty()) {
            //! if identity provides specific configuration group
            if (appletConfigGroup.hasGroup(m_subIdentities[i].cfgGroup)) {
                appletConfigGroup = appletGroup.group(m_subIdentities[i].cfgGroup);
            }
        }

        if (!m_subIdentities[i].cfgProperty.isEmpty()) {
            //! if identity provides specific property for configuration group
            if (appletConfigGroup.hasKey(m_subIdentities[i].cfgProperty)) {
                int subId = appletConfigGroup.readEntry(m_subIdentities[i].cfgProperty, IDNULL);
                return isValid(subId) ? i : IDNULL;
            }
        }
    }

    return IDNULL;
}

Plasma::Containment *Storage::subContainmentOf(const Layout::GenericLayout *layout, const Plasma::Applet *applet)
{
    if (!layout || !applet) {
        return nullptr;
    }

    if (isSubContainment(layout, applet)) {
        for (const auto containment : *layout->containments()) {
            Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(containment->parent());
            if (parentApplet && parentApplet == applet) {
                return containment;
            }
        }
    }

    return nullptr;
}

void Storage::lock(const Layout::GenericLayout *layout)
{
    QFileInfo layoutFileInfo(layout->file());

    if (layoutFileInfo.exists() && layoutFileInfo.isWritable()) {
        QFile(layout->file()).setPermissions(QFileDevice::ReadUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }
}

void Storage::unlock(const Layout::GenericLayout *layout)
{
    QFileInfo layoutFileInfo(layout->file());

    if (layoutFileInfo.exists() && !layoutFileInfo.isWritable()) {
        QFile(layout->file()).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }
}


void Storage::importToCorona(const Layout::GenericLayout *layout)
{
    if (!layout->corona()) {
        return;
    }

    //! Setting mutable for create a containment
    layout->corona()->setImmutability(Plasma::Types::Mutable);

    QString temp1FilePath = m_storageTmpDir.path() +  "/" + layout->name() + ".multiple.views";
    //! we need to copy first the layout file because the kde cache
    //! may not have yet been updated (KSharedConfigPtr)
    //! this way we make sure at the latest changes stored in the layout file
    //! will be also available when changing to Multiple Layouts
    QString tempLayoutFilePath = m_storageTmpDir.path() +  "/" + layout->name() + ".multiple.tmplayout";

    //! WE NEED A WAY TO COPY A CONTAINMENT!!!!
    QFile tempLayoutFile(tempLayoutFilePath);
    QFile copyFile(temp1FilePath);
    QFile layoutOriginalFile(layout->file());

    if (tempLayoutFile.exists()) {
        tempLayoutFile.remove();
    }

    if (copyFile.exists())
        copyFile.remove();

    layoutOriginalFile.copy(tempLayoutFilePath);

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(tempLayoutFilePath);
    KSharedConfigPtr newFile = KSharedConfig::openConfig(temp1FilePath);
    KConfigGroup copyGroup = KConfigGroup(newFile, "Containments");
    KConfigGroup current_containments = KConfigGroup(filePtr, "Containments");

    current_containments.copyTo(&copyGroup);

    copyGroup.sync();

    //! update ids to unique ones
    QString temp2File = newUniqueIdsFile(temp1FilePath, layout);

    //! Finally import the configuration
    importLayoutFile(layout, temp2File);
}


QString Storage::availableId(QStringList all, QStringList assigned, int base)
{
    bool found = false;

    int i = base;

    while (!found && i < 32000) {
        QString iStr = QString::number(i);

        if (!all.contains(iStr) && !assigned.contains(iStr)) {
            return iStr;
        }

        i++;
    }

    return QString("");
}

bool Storage::appletGroupIsValid(const KConfigGroup &appletGroup)
{
    return !( appletGroup.keyList().count() == 0
              && appletGroup.groupList().count() == 1
              && appletGroup.groupList().at(0) == "Configuration"
              && appletGroup.group("Configuration").keyList().count() == 1
              && appletGroup.group("Configuration").hasKey("PreloadWeight") );
}

QStringList Storage::containmentsIds(const QString &filepath)
{
    QStringList ids;

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(filepath);
    KConfigGroup containments = KConfigGroup(filePtr, "Containments");

    for(const auto &cId : containments.groupList()) {
        ids << cId;
    }

    return ids;
}

QStringList Storage::appletsIds(const QString &filepath)
{
    QStringList ids;

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(filepath);
    KConfigGroup containments = KConfigGroup(filePtr, "Containments");

    for(const auto &cId : containments.groupList()) {
        for(const auto &aId : containments.group(cId).group("Applets").groupList()) {
            ids << aId;
        }
    }

    return ids;
}

QString Storage::newUniqueIdsFile(QString originFile, const Layout::GenericLayout *destinationLayout)
{
    if (!destinationLayout) {
        return QString();
    }

    QString currentdestinationname = destinationLayout->name();
    QString currentdestinationfile = "";

    if (!destinationLayout->isActive()) {
        currentdestinationfile = destinationLayout->file();
    }

    QString tempFile = m_storageTmpDir.path() + "/" + currentdestinationname + ".views.newids";

    QFile copyFile(tempFile);

    if (copyFile.exists()) {
        copyFile.remove();
    }

    //! BEGIN updating the ids in the temp file
    QStringList allIds;

    if (destinationLayout->isActive()) {
        allIds << destinationLayout->corona()->containmentsIds();
        allIds << destinationLayout->corona()->appletsIds();
    } else {
        allIds << containmentsIds(currentdestinationfile);
        allIds << appletsIds(currentdestinationfile);
    }

    QStringList toInvestigateContainmentIds;
    QStringList toInvestigateAppletIds;
    QStringList toInvestigateSubContIds;

    //! first is the subcontainment id
    QHash<QString, QString> subParentContainmentIds;
    QHash<QString, QString> subAppletIds;

    //qDebug() << "Ids:" << allIds;

    //qDebug() << "to copy containments: " << toCopyContainmentIds;
    //qDebug() << "to copy applets: " << toCopyAppletIds;

    QStringList assignedIds;
    QHash<QString, QString> assigned;

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(originFile);
    KConfigGroup investigate_conts = KConfigGroup(filePtr, "Containments");

    //! Record the containment and applet ids
    for (const auto &cId : investigate_conts.groupList()) {
        toInvestigateContainmentIds << cId;
        auto appletsEntries = investigate_conts.group(cId).group("Applets");
        toInvestigateAppletIds << appletsEntries.groupList();

        //! investigate for subcontainments
        for (const auto &appletId : appletsEntries.groupList()) {
            int subId = subContainmentId(appletsEntries.group(appletId));

            //! It is a subcontainment !!!
            if (isValid(subId)) {
                QString tSubIdStr = QString::number(subId);
                toInvestigateSubContIds << tSubIdStr;
                subParentContainmentIds[tSubIdStr] = cId;
                subAppletIds[tSubIdStr] = appletId;
                qDebug() << "subcontainment was found in the containment...";
            }
        }
    }

    //! Reassign containment and applet ids to unique ones
    for (const auto &contId : toInvestigateContainmentIds) {        
        QString newId;

        if (contId.toInt()>=12 && !allIds.contains(contId) && !assignedIds.contains(contId)) {
            newId = contId;
        } else {
            newId = availableId(allIds, assignedIds, 12);
        }

        assignedIds << newId;
        assigned[contId] = newId;
    }

    for (const auto &appId : toInvestigateAppletIds) {
        QString newId;

        if (appId.toInt()>=40 && !allIds.contains(appId) && !assignedIds.contains(appId)) {
            newId = appId;
        } else {
            newId = availableId(allIds, assignedIds, 40);
        }

        assignedIds << newId;
        assigned[appId] = newId;
    }

    qDebug() << "ALL CORONA IDS ::: " << allIds;
    qDebug() << "FULL ASSIGNMENTS ::: " << assigned;

    for (const auto &cId : toInvestigateContainmentIds) {
        QString value = assigned[cId];

        if (assigned.contains(value)) {
            QString value2 = assigned[value];

            if (cId != assigned[cId] && !value2.isEmpty() && cId == value2) {
                qDebug() << "PROBLEM APPEARED !!!! FOR :::: " << cId << " .. fixed ..";
                assigned[cId] = cId;
                assigned[value] = value;
            }
        }
    }

    for (const auto &aId : toInvestigateAppletIds) {
        QString value = assigned[aId];

        if (assigned.contains(value)) {
            QString value2 = assigned[value];

            if (aId != assigned[aId] && !value2.isEmpty() && aId == value2) {
                qDebug() << "PROBLEM APPEARED !!!! FOR :::: " << aId << " .. fixed ..";
                assigned[aId] = aId;
                assigned[value] = value;
            }
        }
    }

    qDebug() << "FIXED FULL ASSIGNMENTS ::: " << assigned;

    //! update applet ids in their containment order and in MultipleLayouts update also the layoutId
    for (const auto &cId : investigate_conts.groupList()) {
        //! Update options that contain applet ids
        //! (appletOrder) and (lockedZoomApplets) and (userBlocksColorizingApplets)
        QStringList options;
        options << "appletOrder" << "lockedZoomApplets" << "userBlocksColorizingApplets";

        for (const auto &settingStr : options) {
            QString order1 = investigate_conts.group(cId).group("General").readEntry(settingStr, QString());

            if (!order1.isEmpty()) {
                QStringList order1Ids = order1.split(";");
                QStringList fixedOrder1Ids;

                for (int i = 0; i < order1Ids.count(); ++i) {
                    fixedOrder1Ids.append(assigned[order1Ids[i]]);
                }

                QString fixedOrder1 = fixedOrder1Ids.join(";");
                investigate_conts.group(cId).group("General").writeEntry(settingStr, fixedOrder1);
            }
        }

        if (destinationLayout->isActive() && destinationLayout->corona()->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
            //! will be added in main corona multiple layouts file
            investigate_conts.group(cId).writeEntry("layoutId", destinationLayout->name());
        } else {
            //! will be added in inactive layout
            investigate_conts.group(cId).writeEntry("layoutId", QString());
        }
    }

    //! must update also the sub id in its applet
    for (const auto &subId : toInvestigateSubContIds) {
        KConfigGroup subParentContainment = investigate_conts.group(subParentContainmentIds[subId]);
        KConfigGroup subAppletConfig = subParentContainment.group("Applets").group(subAppletIds[subId]);

        int entityIndex = subIdentityIndex(subAppletConfig);

        if (entityIndex >= 0) {
            if (!m_subIdentities[entityIndex].cfgGroup.isEmpty()) {
                subAppletConfig = subAppletConfig.group(m_subIdentities[entityIndex].cfgGroup);
            }

            if (!m_subIdentities[entityIndex].cfgProperty.isEmpty()) {
                subAppletConfig.writeEntry(m_subIdentities[entityIndex].cfgProperty, assigned[subId]);
                subParentContainment.sync();
            }
        }
    }

    investigate_conts.sync();

    //! Copy To Temp 2 File And Update Correctly The Ids
    KSharedConfigPtr file2Ptr = KSharedConfig::openConfig(tempFile);
    KConfigGroup fixedNewContainmets = KConfigGroup(file2Ptr, "Containments");

    for (const auto &contId : investigate_conts.groupList()) {
        QString pluginId = investigate_conts.group(contId).readEntry("plugin", "");

        if (pluginId != "org.kde.desktopcontainment") { //!don't add ghost containments
            KConfigGroup newContainmentGroup = fixedNewContainmets.group(assigned[contId]);
            investigate_conts.group(contId).copyTo(&newContainmentGroup);

            newContainmentGroup.group("Applets").deleteGroup();

            for (const auto &appId : investigate_conts.group(contId).group("Applets").groupList()) {
                KConfigGroup appletGroup = investigate_conts.group(contId).group("Applets").group(appId);
                KConfigGroup newAppletGroup = fixedNewContainmets.group(assigned[contId]).group("Applets").group(assigned[appId]);
                appletGroup.copyTo(&newAppletGroup);
            }
        }
    }

    fixedNewContainmets.sync();

    return tempFile;
}

void Storage::syncToLayoutFile(const Layout::GenericLayout *layout, bool removeLayoutId)
{
    if (!layout->corona() || !isWritable(layout)) {
        return;
    }

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(layout->file());

    KConfigGroup oldContainments = KConfigGroup(filePtr, "Containments");
    oldContainments.deleteGroup();

    qDebug() << " LAYOUT :: " << layout->name() << " is syncing its original file.";

    for (const auto containment : *layout->containments()) {
        if (removeLayoutId) {
            containment->config().writeEntry("layoutId", "");
        }

        KConfigGroup newGroup = oldContainments.group(QString::number(containment->id()));
        containment->config().copyTo(&newGroup);

        if (!removeLayoutId) {
            newGroup.writeEntry("layoutId", "");
        }

        newGroup.sync();
    }

    oldContainments.sync();
}

QList<Plasma::Containment *> Storage::importLayoutFile(const Layout::GenericLayout *layout, QString file)
{
    KSharedConfigPtr filePtr = KSharedConfig::openConfig(file);
    auto newContainments = layout->corona()->importLayout(KConfigGroup(filePtr, ""));

    qDebug() << " imported containments ::: " << newContainments.length();

    QList<Plasma::Containment *> importedViews;

    for (const auto containment : newContainments) {
        if (isLatteContainment(containment)) {
            qDebug() << "new latte containment id: " << containment->id();
            importedViews << containment;
        }
    }

    return importedViews;
}

void Storage::importContainments(const QString &originFile, const QString &destinationFile)
{
    if (originFile.isEmpty() || destinationFile.isEmpty()) {
        return;
    }

    KSharedConfigPtr originPtr = KSharedConfig::openConfig(originFile);
    KSharedConfigPtr destinationPtr = KSharedConfig::openConfig(destinationFile);

    KConfigGroup originContainments = KConfigGroup(originPtr, "Containments");
    KConfigGroup destinationContainments = KConfigGroup(destinationPtr, "Containments");

    for (const auto originContId : originContainments.groupList()) {
        KConfigGroup destinationContainment(&destinationContainments, originContId);
        originContainments.group(originContId).copyTo(&destinationContainment);
    }

    destinationContainments.sync();
}

Data::View Storage::newView(const Layout::GenericLayout *destinationLayout, const Data::View &nextViewData)
{
    if (!destinationLayout || nextViewData.originFile().isEmpty()) {
        return Data::View();
    }

    qDebug() << "new view for layout";

    if (destinationLayout->isActive()) {
        //! Setting mutable for create a containment
        destinationLayout->corona()->setImmutability(Plasma::Types::Mutable);
    }

    QString templateFile = nextViewData.originFile();
    //! copy view template path in temp file
    QString templateTmpAbsolutePath = m_storageTmpDir.path() + "/" + QFileInfo(templateFile).fileName() + ".newids";

    if (QFile(templateTmpAbsolutePath).exists()) {
        QFile(templateTmpAbsolutePath).remove();
    }

    QFile(templateFile).copy(templateTmpAbsolutePath);

    //! update ids to unique ones
    QString temp2File = newUniqueIdsFile(templateTmpAbsolutePath, destinationLayout);

    //! update view containment data in case next data are provided
    if (nextViewData.state() != Data::View::IsInvalid) {

        KSharedConfigPtr lFile = KSharedConfig::openConfig(temp2File);
        KConfigGroup containments = KConfigGroup(lFile, "Containments");

        for (const auto cId : containments.groupList()) {
            if (Layouts::Storage::self()->isLatteContainment(containments.group(cId))) {
                //! first view we will find, we update its value
                updateView(containments.group(cId), nextViewData);
                break;
            }
        }
    }

    Data::ViewsTable updatedNextViews = views(temp2File);

    if (updatedNextViews.rowCount() <= 0) {
        return Data::View();
    }

    if (destinationLayout->isActive()) {
        //! import views for active layout
        QList<Plasma::Containment *> importedViews = importLayoutFile(destinationLayout, temp2File);

        Plasma::Containment *newContainment = (importedViews.size() == 1 ? importedViews[0] : nullptr);

        if (!newContainment || !newContainment->kPackage().isValid()) {
            qWarning() << "the requested containment plugin can not be located or loaded from:" << templateFile;
            return Data::View();
        }
    } else {
        //! import views for inactive layout
        importContainments(temp2File, destinationLayout->file());
    }

    return updatedNextViews[0];
}

void Storage::clearExportedLayoutSettings(KConfigGroup &layoutSettingsGroup)
{
    layoutSettingsGroup.writeEntry("preferredForShortcutsTouched", false);
    layoutSettingsGroup.writeEntry("lastUsedActivity", QString());
    layoutSettingsGroup.writeEntry("activities", QStringList());
    layoutSettingsGroup.sync();
}

bool Storage::exportTemplate(const QString &originFile, const QString &destinationFile,const Data::AppletsTable &approvedApplets)
{
    if (originFile.isEmpty() || !QFile(originFile).exists() || destinationFile.isEmpty()) {
        return false;
    }

    if (QFile(destinationFile).exists()) {
        QFile::remove(destinationFile);
    }

    QFile(originFile).copy(destinationFile);

    KSharedConfigPtr destFilePtr = KSharedConfig::openConfig(destinationFile);
    KConfigGroup containments = KConfigGroup(destFilePtr, "Containments");

    QStringList rejectedSubContainments;

    //! clear applets that are not approved
    for (const auto &cId : containments.groupList()) {
        //! clear properties
        containments.group(cId).writeEntry("layoutId", QString());
        if (isLatteContainment(containments.group(cId))) {
            containments.group(cId).writeEntry("isPreferredForShortcuts", false);
        }

        //! clear applets
        auto applets = containments.group(cId).group("Applets");
        for (const auto &aId: applets.groupList()) {
            QString pluginId = applets.group(aId).readEntry("plugin", "");

            if (!approvedApplets.containsId(pluginId)) {
                if (!isSubContainment(applets.group(aId))) {
                    //!remove all configuration for that applet
                    for (const auto &configId: applets.group(aId).groupList()) {
                        applets.group(aId).group(configId).deleteGroup();
                    }
                } else {
                    //! register which subcontaiments should return to default properties
                    rejectedSubContainments << QString::number(subContainmentId(applets.group(aId)));
                }
            }
        }
    }

    //! clear rejected SubContainments
    for (const auto &cId : containments.groupList()) {
        if (rejectedSubContainments.contains(cId)) {
            containments.group(cId).group("General").deleteGroup();
        }
    };

    KConfigGroup layoutSettingsGrp(destFilePtr, "LayoutSettings");
    clearExportedLayoutSettings(layoutSettingsGrp);
    containments.sync();

    return true;
}

bool Storage::exportTemplate(const Layout::GenericLayout *layout, Plasma::Containment *containment, const QString &destinationFile, const Data::AppletsTable &approvedApplets)
{
    if (!layout || !containment || destinationFile.isEmpty()) {
        return false;
    }

    if (QFile(destinationFile).exists()) {
        QFile::remove(destinationFile);
    }

    KSharedConfigPtr destFilePtr = KSharedConfig::openConfig(destinationFile);
    KConfigGroup copied_conts = KConfigGroup(destFilePtr, "Containments");
    KConfigGroup copied_c1 = KConfigGroup(&copied_conts, QString::number(containment->id()));

    containment->config().copyTo(&copied_c1);

    //!investigate if there are subcontainments in the containment to copy also

    //! subId, subAppletId
    QHash<uint, QString> subInfo;
    auto applets = containment->config().group("Applets");

    for (const auto &applet : applets.groupList()) {
        int tSubId = subContainmentId(applets.group(applet));

        //! It is a subcontainment !!!
        if (isValid(tSubId)) {
            subInfo[tSubId] = applet;
            qDebug() << "subcontainment with id "<< tSubId << " was found in the containment... ::: " << containment->id();
        }
    }

    if (subInfo.count() > 0) {
        for(const auto subId : subInfo.keys()) {
            Plasma::Containment *subcontainment{nullptr};

            for (const auto containment : layout->corona()->containments()) {
                if (containment->id() == subId) {
                    subcontainment = containment;
                    break;
                }
            }

            if (subcontainment) {
                KConfigGroup copied_sub = KConfigGroup(&copied_conts, QString::number(subcontainment->id()));
                subcontainment->config().copyTo(&copied_sub);
            }
        }
    }
    //! end of subcontainments specific code

    QStringList rejectedSubContainments;

    //! clear applets that are not approved
    for (const auto &cId : copied_conts.groupList()) {
        //! clear properties
        copied_conts.group(cId).writeEntry("layoutId", QString());
        if (isLatteContainment(copied_conts.group(cId))) {
            copied_conts.group(cId).writeEntry("isPreferredForShortcuts", false);
        }

        //! clear applets
        auto applets = copied_conts.group(cId).group("Applets");
        for (const auto &aId: applets.groupList()) {
            QString pluginId = applets.group(aId).readEntry("plugin", "");

            if (!approvedApplets.containsId(pluginId)) {
                if (!isSubContainment(applets.group(aId))) {
                    //!remove all configuration for that applet
                    for (const auto &configId: applets.group(aId).groupList()) {
                        applets.group(aId).group(configId).deleteGroup();
                    }
                } else {
                    //! register which subcontaiments should return to default properties
                    rejectedSubContainments << QString::number(subContainmentId(applets.group(aId)));
                }
            }
        }
    }

    //! clear rejected SubContainments
    for (const auto &cId : copied_conts.groupList()) {
        if (rejectedSubContainments.contains(cId)) {
            copied_conts.group(cId).group("General").deleteGroup();
        }
    };

    KConfigGroup layoutSettingsGrp(destFilePtr, "LayoutSettings");
    clearExportedLayoutSettings(layoutSettingsGrp);
    copied_conts.sync();

    return true;
}

bool Storage::hasDifferentAppletsWithSameId(const Layout::GenericLayout *layout, Data::Error &error)
{
    if (!layout  || layout->file().isEmpty() || !QFile(layout->file()).exists()) {
        return false;
    }

    error.id = m_knownErrors[Data::Error::APPLETSWITHSAMEID].id;
    error.name = m_knownErrors[Data::Error::APPLETSWITHSAMEID].name;

    if (layout->isActive()) { // active layout
        QStringList registeredapplets;
        QStringList conflictedapplets;

        //! split ids to normal registered and conflicted
        for (const auto containment : *layout->containments()) {
            QString cid = QString::number(containment->id());

            for (const auto applet : containment->applets()) {
                QString aid = QString::number(applet->id());

                if (!registeredapplets.contains(aid)) {
                    registeredapplets << aid;
                } else if (!conflictedapplets.contains(aid)) {
                    conflictedapplets << aid;
                }
            }
        }

        //! create error data
        for (const auto containment : *layout->containments()) {
            QString cid = QString::number(containment->id());

            for (const auto applet : containment->applets()) {
                QString aid = QString::number(applet->id());

                if (!conflictedapplets.contains(aid)) {
                   continue;
                }

                Data::ErrorInformation errorinfo;
                errorinfo.containment = metadata(containment->pluginMetaData().pluginId());
                errorinfo.containment.storageId = cid;
                errorinfo.applet = metadata(applet->pluginMetaData().pluginId());
                errorinfo.applet.storageId = aid;

                error.information << errorinfo;
            }
        }
    } else { // inactive layout
        KSharedConfigPtr lfile = KSharedConfig::openConfig(layout->file());
        KConfigGroup containmentsEntries = KConfigGroup(lfile, "Containments");

        QStringList registeredapplets;
        QStringList conflictedapplets;

        //! split ids to normal registered and conflicted
        for (const auto &cid : containmentsEntries.groupList()) {
            for (const auto &aid : containmentsEntries.group(cid).group("Applets").groupList()) {
                if (!registeredapplets.contains(aid)) {
                    registeredapplets << aid;
                } else if (!conflictedapplets.contains(aid)) {
                    conflictedapplets << aid;
                }
            }
        }

        //! create error data
        for (const auto &cid : containmentsEntries.groupList()) {
            for (const auto &aid : containmentsEntries.group(cid).group("Applets").groupList()) {
                if (!conflictedapplets.contains(aid)) {
                   continue;
                }

                Data::ErrorInformation errorinfo;
                errorinfo.containment = metadata(containmentsEntries.group(cid).readEntry("plugin", ""));
                errorinfo.containment.storageId = cid;
                errorinfo.applet = metadata(containmentsEntries.group(cid).group("Applets").group(aid).readEntry("plugin", ""));
                errorinfo.applet.storageId = aid;

                error.information << errorinfo;
            }
        }
    }

    return !error.information.isEmpty();
}

bool Storage::hasAppletsAndContainmentsWithSameId(const Layout::GenericLayout *layout, Data::Warning &warning)
{
    if (!layout  || layout->file().isEmpty() || !QFile(layout->file()).exists()) {
        return false;
    }

    warning.id = m_knownErrors[Data::Error::APPLETSANDCONTAINMENTSWITHSAMEID].id;
    warning.name = m_knownErrors[Data::Error::APPLETSANDCONTAINMENTSWITHSAMEID].name;

    if (layout->isActive()) { // active layout
        QStringList registeredcontainments;
        QStringList conflicted;

        //! discover normal containment ids
        for (const auto containment : *layout->containments()) {
            QString cid = QString::number(containment->id());

            if (registeredcontainments.contains(cid)) {
                continue;
            }

            registeredcontainments << cid;
        }

        //! discover conflicted ids between containments and applets
        for (const auto containment : *layout->containments()) {
            QString cid = QString::number(containment->id());

            for (const auto applet : containment->applets()) {
                QString aid = QString::number(applet->id());

                if (!registeredcontainments.contains(aid)) {
                    continue;
                } else if (!conflicted.contains(aid)) {
                    conflicted << aid;
                }
            }
        }

        //! create warning data
        for (const auto containment : *layout->containments()) {
            QString cid = QString::number(containment->id());

            if (conflicted.contains(cid)) {
                Data::WarningInformation warninginfo;
                warninginfo.containment = metadata(containment->pluginMetaData().pluginId());
                warninginfo.containment.storageId = cid;

                warning.information << warninginfo;
            }

            for (const auto applet : containment->applets()) {
                QString aid = QString::number(applet->id());

                if (!conflicted.contains(aid)) {
                   continue;
                }

                Data::WarningInformation warninginfo;
                warninginfo.containment = metadata(containment->pluginMetaData().pluginId());
                warninginfo.containment.storageId = cid;
                warninginfo.applet = metadata(applet->pluginMetaData().pluginId());
                warninginfo.applet.storageId = aid;

                warning.information << warninginfo;
            }
        }
    } else { // inactive layout
        KSharedConfigPtr lfile = KSharedConfig::openConfig(layout->file());
        KConfigGroup containmentsEntries = KConfigGroup(lfile, "Containments");

        QStringList registeredcontainments;
        QStringList conflicted;

        //! discover normal containment ids
        for (const auto &cid : containmentsEntries.groupList()) {
            if (registeredcontainments.contains(cid)) {
                continue;
            }

            registeredcontainments << cid;
        }

        //! discover conflicted ids between containments and applets
        for (const auto &cid : containmentsEntries.groupList()) {
            for (const auto &aid : containmentsEntries.group(cid).group("Applets").groupList()) {
                if (!registeredcontainments.contains(aid)) {
                    continue;
                } else if (!conflicted.contains(aid)) {
                    conflicted << aid;
                }
            }
        }

        //! create warning data
        for (const auto &cid : containmentsEntries.groupList()) {
            if (conflicted.contains(cid)) {
                Data::WarningInformation warninginfo;
                warninginfo.containment = metadata(containmentsEntries.group(cid).readEntry("plugin", ""));
                warninginfo.containment.storageId = cid;

                warning.information << warninginfo;
            }

            for (const auto &aid : containmentsEntries.group(cid).group("Applets").groupList()) {
                if (!conflicted.contains(aid)) {
                   continue;
                }

                Data::WarningInformation warninginfo;
                warninginfo.containment = metadata(containmentsEntries.group(cid).readEntry("plugin", ""));
                warninginfo.containment.storageId = cid;
                warninginfo.applet = metadata(containmentsEntries.group(cid).group("Applets").group(aid).readEntry("plugin", ""));
                warninginfo.applet.storageId = aid;

                warning.information << warninginfo;
            }
        }
    }

    return !warning.information.isEmpty();
}

bool Storage::hasOrphanedSubContainments(const Layout::GenericLayout *layout, Data::Warning &warning)
{
    if (!layout  || layout->file().isEmpty() || !QFile(layout->file()).exists()) {
        return false;
    }

    warning.id = m_knownErrors[Data::Error::ORPHANEDSUBCONTAINMENTS].id;
    warning.name = m_knownErrors[Data::Error::ORPHANEDSUBCONTAINMENTS].name;

    Data::ViewsTable views = Layouts::Storage::self()->views(layout);

    if (layout->isActive()) { // active layout
        //! create warning data
        for (const auto containment : *layout->containments()) {
            QString cid = QString::number(containment->id());

            if (views.hasContainmentId(cid)) {
                continue;
            }

            Data::WarningInformation warninginfo;
            warninginfo.containment = metadata(containment->pluginMetaData().pluginId());
            warninginfo.containment.storageId = cid;
            warning.information << warninginfo;
        }

    } else { // inactive layout
        KSharedConfigPtr lfile = KSharedConfig::openConfig(layout->file());
        KConfigGroup containmentsEntries = KConfigGroup(lfile, "Containments");

        //! create warning data
        for (const auto &cid : containmentsEntries.groupList()) {
            if (views.hasContainmentId(cid)) {
                continue;
            }

            Data::WarningInformation warninginfo;
            warninginfo.containment = metadata(containmentsEntries.group(cid).readEntry("plugin", ""));
            warninginfo.containment.storageId = cid;
            warning.information << warninginfo;
        }
    }

    return !warning.information.isEmpty();
}

Data::ErrorsList Storage::errors(const Layout::GenericLayout *layout)
{
    Data::ErrorsList errs;

    if (!layout  || layout->file().isEmpty() || !QFile(layout->file()).exists()) {
        return errs;
    }

    Data::Error error1;

    if (hasDifferentAppletsWithSameId(layout, error1)) {
        errs << error1;
    }

    return errs;
}

Data::WarningsList Storage::warnings(const Layout::GenericLayout *layout)
{
    Data::WarningsList warns;

    if (!layout  || layout->file().isEmpty() || !QFile(layout->file()).exists()) {
        return warns;
    }

    Data::Warning warning1;

    if (hasAppletsAndContainmentsWithSameId(layout, warning1)) {
        warns << warning1;
    }

    Data::Warning warning2;

    if (hasOrphanedSubContainments(layout, warning2)) {
        warns << warning2;
    }

    return warns;
}


bool Storage::isBroken(const Layout::GenericLayout *layout, QStringList &errors) const
{
    if (layout->file().isEmpty() || !QFile(layout->file()).exists()) {
        return false;
    }

    QStringList ids;
    QStringList conts;
    QStringList applets;

    KSharedConfigPtr lFile = KSharedConfig::openConfig(layout->file());

    if (!layout->corona()) {
        KConfigGroup containmentsEntries = KConfigGroup(lFile, "Containments");
        ids << containmentsEntries.groupList();
        conts << ids;

        for (const auto &cId : containmentsEntries.groupList()) {
            auto appletsEntries = containmentsEntries.group(cId).group("Applets");

            QStringList validAppletIds;
            bool updated{false};

            for (const auto &appletId : appletsEntries.groupList()) {
                KConfigGroup appletGroup = appletsEntries.group(appletId);

                if (Layouts::Storage::appletGroupIsValid(appletGroup)) {
                    validAppletIds << appletId;
                } else {
                    updated = true;
                    //! heal layout file by removing applet config records that are not used any more
                    qDebug() << "Layout: " << layout->name() << " removing deprecated applet : " << appletId;
                    appletsEntries.deleteGroup(appletId);
                }
            }

            if (updated) {
                appletsEntries.sync();
            }

            ids << validAppletIds;
            applets << validAppletIds;
        }
    } else {
        for (const auto containment : *layout->containments()) {
            ids << QString::number(containment->id());
            conts << QString::number(containment->id());

            for (const auto applet : containment->applets()) {
                ids << QString::number(applet->id());
                applets << QString::number(applet->id());
            }
        }
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QSet<QString> idsSet = QSet<QString>::fromList(ids);
#else
    QSet<QString> idsSet(ids.begin(), ids.end());
#endif
    /* a different way to count duplicates
    QMap<QString, int> countOfStrings;

    for (int i = 0; i < ids.count(); i++) {
        countOfStrings[ids[i]]++;
    }*/

    if (idsSet.count() != ids.count()) {
        qDebug() << "   ----   ERROR - BROKEN LAYOUT :: " << layout->name() << " ----";

        if (!layout->corona()) {
            qDebug() << "   --- storaged file : " << layout->file();
        } else {
            if (layout->corona()->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
                qDebug() << "   --- in multiple layouts hidden file : " << Layouts::Importer::layoutUserFilePath(Layout::MULTIPLELAYOUTSHIDDENNAME);
            } else {
                qDebug() << "   --- in active layout file : " << layout->file();
            }
        }

        qDebug() << "Containments :: " << conts;
        qDebug() << "Applets :: " << applets;

        for (const QString &c : conts) {
            if (applets.contains(c)) {
                QString errorStr = i18n("Same applet and containment id found ::: ") + c;
                qDebug() << "Error: " << errorStr;
                errors << errorStr;
            }
        }

        for (int i = 0; i < ids.count(); ++i) {
            for (int j = i + 1; j < ids.count(); ++j) {
                if (ids[i] == ids[j]) {
                    QString errorStr = i18n("Different applets with same id ::: ") + ids[i];
                    qDebug() << "Error: " << errorStr;
                    errors << errorStr;
                }
            }
        }

        qDebug() << "  -- - -- - -- - -- - - -- - - - - -- - - - - ";

        if (!layout->corona()) {
            KConfigGroup containmentsEntries = KConfigGroup(lFile, "Containments");

            for (const auto &cId : containmentsEntries.groupList()) {
                auto appletsEntries = containmentsEntries.group(cId).group("Applets");

                qDebug() << " CONTAINMENT : " << cId << " APPLETS : " << appletsEntries.groupList();
            }
        } else {
            for (const auto containment : *layout->containments()) {
                QStringList appletsIds;

                for (const auto applet : containment->applets()) {
                    appletsIds << QString::number(applet->id());
                }

                qDebug() << " CONTAINMENT : " << containment->id() << " APPLETS : " << appletsIds.join(",");
            }
        }

        return true;
    }

    return false;
}

//! AppletsData Information
Data::Applet Storage::metadata(const QString &pluginId)
{
    Data::Applet data;
    data.id = pluginId;

    KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"));
    pkg.setDefaultPackageRoot(QStringLiteral("plasma/plasmoids"));
    pkg.setPath(pluginId);

    if (pkg.isValid()) {
        data.name = pkg.metadata().name();
        data.description = pkg.metadata().description();

        QString iconName = pkg.metadata().iconName();
        if (!iconName.startsWith("/") && iconName.contains("/")) {
            data.icon = QFileInfo(pkg.metadata().fileName()).absolutePath() + "/" + iconName;
        } else {
            data.icon = iconName;
        }
    }

    if (data.name.isEmpty()) {
        //! this is also a way to identify if a package is installed or not in current system
        data.name = data.id;
    }

    return data;
}

Data::AppletsTable Storage::plugins(const Layout::GenericLayout *layout, const int containmentid)
{
    Data::AppletsTable knownapplets;
    Data::AppletsTable unknownapplets;

    if (!layout) {
        return knownapplets;
    }

    //! empty means all containments are valid
    QList<int> validcontainmentids;

    if (isValid(containmentid)) {
        validcontainmentids << containmentid;

        //! searching for specific containment and subcontainments and ignore all other containments
        for(auto containment : *layout->containments()) {
            if (((int)containment->id()) != containmentid) {
                //! ignore irrelevant containments
                continue;
            }

            for (auto applet : containment->applets()) {
                if (isSubContainment(layout, applet)) {
                    validcontainmentids << subContainmentId(applet->config());
                }
            }
        }
    }

    //! cycle through valid contaiments in order to retrieve their metadata
    for(auto containment : *layout->containments()) {
        if (validcontainmentids.count()>0 && !validcontainmentids.contains(containment->id())) {
            //! searching only for valid containments
            continue;
        }

        for (auto applet : containment->applets()) {
            QString pluginId = applet->pluginMetaData().pluginId();
            if (!knownapplets.containsId(pluginId) && !unknownapplets.containsId(pluginId)) {
                Data::Applet appletdata = metadata(pluginId);

                if (appletdata.isInstalled()) {
                    knownapplets.insertBasedOnName(appletdata);
                } else if (appletdata.isValid()) {
                    unknownapplets.insertBasedOnName(appletdata);
                }
            }
        }
    }

    knownapplets << unknownapplets;

    return knownapplets;
}

Data::AppletsTable Storage::plugins(const QString &layoutfile, const int containmentid)
{
    Data::AppletsTable knownapplets;
    Data::AppletsTable unknownapplets;

    if (layoutfile.isEmpty()) {
        return knownapplets;
    }

    KSharedConfigPtr lFile = KSharedConfig::openConfig(layoutfile);
    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    //! empty means all containments are valid
    QList<int> validcontainmentids;

    if (isValid(containmentid)) {
        validcontainmentids << containmentid;

        //! searching for specific containment and subcontainments and ignore all other containments
        for (const auto &cId : containmentGroups.groupList()) {
            if (cId.toInt() != containmentid) {
                //! ignore irrelevant containments
                continue;
            }

            auto appletGroups = containmentGroups.group(cId).group("Applets");

            for (const auto &appletId : appletGroups.groupList()) {
                KConfigGroup appletCfg = appletGroups.group(appletId);
                if (isSubContainment(appletCfg)) {
                    validcontainmentids << subContainmentId(appletCfg);
                }
            }
        }
    }

    //! cycle through valid contaiments in order to retrieve their metadata
    for (const auto &cId : containmentGroups.groupList()) {
        if (validcontainmentids.count()>0 && !validcontainmentids.contains(cId.toInt())) {
            //! searching only for valid containments
            continue;
        }

        auto appletGroups = containmentGroups.group(cId).group("Applets");

        for (const auto &appletId : appletGroups.groupList()) {
            KConfigGroup appletCfg = appletGroups.group(appletId);
            QString pluginId = appletCfg.readEntry("plugin", "");

            if (!knownapplets.containsId(pluginId) && !unknownapplets.containsId(pluginId)) {
                Data::Applet appletdata = metadata(pluginId);

                if (appletdata.isInstalled()) {
                    knownapplets.insertBasedOnName(appletdata);
                } else if (appletdata.isValid()) {
                    unknownapplets.insertBasedOnName(appletdata);
                }
            }
        }
    }

    knownapplets << unknownapplets;

    return knownapplets;
}

//! Views Data

void Storage::syncContainmentConfig(Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    for(auto applet: containment->applets()) {
        KConfigGroup appletGeneralConfig = applet->config().group("General");

        if (appletGeneralConfig.exists()) {
            appletGeneralConfig.sync();
        }

        applet->config().sync();
    }

    containment->config().sync();
}

bool Storage::containsView(const QString &filepath, const int &viewId)
{
    KSharedConfigPtr lFile = KSharedConfig::openConfig(filepath);
    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");
    KConfigGroup viewGroup = containmentGroups.group(QString::number(viewId));
    return viewGroup.exists() && isLatteContainment(viewGroup);
}

Data::GenericTable<Data::Generic> Storage::subcontainments(const Layout::GenericLayout *layout, const Plasma::Containment *lattecontainment) const
{
    Data::GenericTable<Data::Generic> subs;

    if (!layout || !Layouts::Storage::self()->isLatteContainment(lattecontainment)) {
        return subs;
    }

    for (const auto containment : (*layout->containments())) {
        if (containment == lattecontainment) {
            continue;
        }

        Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(containment->parent());

        //! add subcontainments for that lattecontainment
        if (parentApplet && parentApplet->containment() && parentApplet->containment() == lattecontainment) {
            Data::Generic subdata;
            subdata.id = QString::number(containment->id());
            subs << subdata;
        }
    }

    return subs;
}

Data::GenericTable<Data::Generic> Storage::subcontainments(const KConfigGroup &containmentGroup)
{
    Data::GenericTable<Data::Generic> subs;

    if (!Layouts::Storage::self()->isLatteContainment(containmentGroup)) {
        return subs;
    }

    auto applets = containmentGroup.group("Applets");

    for (const auto &applet : applets.groupList()) {
        if (isSubContainment(applets.group(applet))) {
            Data::Generic subdata;
            subdata.id = QString::number(subContainmentId(applets.group(applet)));
            subs << subdata;
        }
    }

    return subs;
}

Data::View Storage::view(const Layout::GenericLayout *layout, const Plasma::Containment *lattecontainment)
{
    Data::View vdata;

    if (!layout || !Layouts::Storage::self()->isLatteContainment(lattecontainment)) {
        return vdata;
    }

    vdata = view(lattecontainment->config());

    vdata.screen = lattecontainment->screen();
    if (!isValid(vdata.screen)) {
        vdata.screen = lattecontainment->lastScreen();
    }

    vdata.subcontainments = subcontainments(layout, lattecontainment);

    return vdata;
}

Data::View Storage::view(const KConfigGroup &containmentGroup)
{
    Data::View vdata;

    if (!Layouts::Storage::self()->isLatteContainment(containmentGroup)) {
        return vdata;
    }

    vdata.id = containmentGroup.name();
    vdata.name = containmentGroup.readEntry("name", QString());
    vdata.isActive = false;
    vdata.onPrimary = containmentGroup.readEntry("onPrimary", true);
    vdata.screen = containmentGroup.readEntry("lastScreen", IDNULL);
    vdata.screenEdgeMargin = containmentGroup.group("General").readEntry("screenEdgeMargin", (int)0);

    int location = containmentGroup.readEntry("location", (int)Plasma::Types::BottomEdge);
    vdata.edge = (Plasma::Types::Location)location;

    vdata.maxLength = containmentGroup.group("General").readEntry("maxLength", (float)100.0);

    int alignment = containmentGroup.group("General").readEntry("alignment", (int)Latte::Types::Center) ;
    vdata.alignment = (Latte::Types::Alignment)alignment;

    vdata.subcontainments = subcontainments(containmentGroup);
    vdata.setState(Data::View::IsCreated);

    return vdata;
}

void Storage::updateView(KConfigGroup viewGroup, const Data::View &viewData)
{
    if (!Layouts::Storage::self()->isLatteContainment(viewGroup)) {
        return;
    }

    viewGroup.writeEntry("name", viewData.name);
    viewGroup.writeEntry("onPrimary", viewData.onPrimary);
    viewGroup.writeEntry("lastScreen", viewData.screen);
    viewGroup.group("General").writeEntry("screenEdgeMargin", viewData.screenEdgeMargin);
    viewGroup.writeEntry("location", (int)viewData.edge);
    viewGroup.writeEntry("maxLength", viewData.maxLength);
    viewGroup.group("General").writeEntry("alignment", (int)viewData.alignment);
    viewGroup.sync();
}

void Storage::updateView(const Layout::GenericLayout *layout, const Data::View &viewData)
{
    if (!layout) {
        return;
    }

    auto view = layout->viewForContainment(viewData.id.toUInt());

    if (view) {
        qDebug() << "Storage::updateView should not be called because view is active and present...";
        return;
    }

    if (layout->isActive()) {
        //! active view but is not present in active screens;
        auto containment = layout->containmentForId(viewData.id.toUInt());
        if (containment) {
            //! update containment
            containment->setLocation(viewData.edge);
            updateView(containment->config(), viewData);
        }
    } else {
        //! inactive view and in layout storage
        KSharedConfigPtr lFile = KSharedConfig::openConfig(layout->file());
        KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");
        KConfigGroup viewContainment = containmentGroups.group(viewData.id);

        if (viewContainment.exists() && Layouts::Storage::self()->isLatteContainment(viewContainment)) {
            updateView(viewContainment, viewData);
        }
    }
}

void Storage::removeView(const QString &filepath, const Data::View &viewData)
{
    if (!viewData.isValid()) {
        return;
    }

    KSharedConfigPtr lFile = KSharedConfig::openConfig(filepath);
    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    containmentGroups.group(viewData.id).deleteGroup();

    for (int i=0; i<viewData.subcontainments.rowCount(); ++i) {
        containmentGroups.group(viewData.subcontainments[i].id).deleteGroup();
    }

    containmentGroups.sync();
}

QString Storage::storedView(const Layout::GenericLayout *layout, const int &containmentId)
{
    //! make sure that layout and containmentId are valid
    if (!layout) {
        return QString();
    }

    if (layout->isActive()) {
        auto containment = layout->containmentForId((uint)containmentId);
        if (!containment || !isLatteContainment(containment)) {
            return QString();
        }
    } else {
        if (!containsView(layout->file(), containmentId)) {
            return QString();
        }
    }

    //! at this point we are sure that both layout and containmentId are acceptable
    QString nextTmpStoredViewAbsolutePath = m_storageTmpDir.path() + "/" + QFileInfo(layout->name()).fileName() + "." + QString::number(containmentId) + ".stored.tmp";

    QFile tempStoredViewFile(nextTmpStoredViewAbsolutePath);

    if (tempStoredViewFile.exists()) {
        tempStoredViewFile.remove();
    }

    KSharedConfigPtr destinationPtr = KSharedConfig::openConfig(nextTmpStoredViewAbsolutePath);
    KConfigGroup destinationContainments = KConfigGroup(destinationPtr, "Containments");

    if (layout->isActive()) {
        //! update and copy containments
        auto containment = layout->containmentForId((uint)containmentId);
        syncContainmentConfig(containment);

        KConfigGroup destinationViewContainment(&destinationContainments, QString::number(containment->id()));
        containment->config().copyTo(&destinationViewContainment);

        QList<Plasma::Containment *> subconts = layout->subContainmentsOf(containment->id());

        for(const auto subcont : subconts) {
            syncContainmentConfig(subcont);
            KConfigGroup destinationsubcontainment(&destinationContainments, QString::number(subcont->id()));
            subcont->config().copyTo(&destinationsubcontainment);
        }

        //! update with latest view data if active view is present
        auto view = layout->viewForContainment(containment);

        if (view) {
            Data::View currentviewdata = view->data();
            updateView(destinationViewContainment, currentviewdata);
        }
    } else {
        QString containmentid = QString::number(containmentId);
        KConfigGroup destinationViewContainment(&destinationContainments, containmentid);

        KSharedConfigPtr originPtr = KSharedConfig::openConfig(layout->file());
        KConfigGroup originContainments = KConfigGroup(originPtr, "Containments");

        originContainments.group(containmentid).copyTo(&destinationViewContainment);

        Data::GenericTable<Data::Generic> subconts = subcontainments(originContainments.group(containmentid));

        for(int i=0; i<subconts.rowCount(); ++i) {
            QString subid = subconts[i].id;
            KConfigGroup destinationsubcontainment(&destinationContainments, subid);
            originContainments.group(subid).copyTo(&destinationsubcontainment);
        }
    }

    destinationContainments.sync();
    return nextTmpStoredViewAbsolutePath;
}

Data::ViewsTable Storage::views(const Layout::GenericLayout *layout)
{
    Data::ViewsTable vtable;

    if (!layout) {
        return vtable;
    } else if (!layout->isActive()) {
        return views(layout->file());
    }

    for (const auto containment : (*layout->containments())) {
        if (!isLatteContainment(containment)) {
            continue;
        }

        Latte::View *vw = layout->viewForContainment(containment);

        if (vw) {
            vtable << vw->data();
        } else {
            vtable << view(layout, containment);
        }
    }

    return vtable;
}

Data::ViewsTable Storage::views(const QString &file)
{
    Data::ViewsTable vtable;

    KSharedConfigPtr lFile = KSharedConfig::openConfig(file);
    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    for (const auto &cId : containmentGroups.groupList()) {
        if (Layouts::Storage::self()->isLatteContainment(containmentGroups.group(cId))) {
            vtable << view(containmentGroups.group(cId));
        }
    }

    return vtable;
}

QList<int> Storage::viewsScreens(const QString &file)
{
    QList<int> screens;

    KSharedConfigPtr lFile = KSharedConfig::openConfig(file);

    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    for (const auto &cId : containmentGroups.groupList()) {
        if (Layouts::Storage::self()->isLatteContainment(containmentGroups.group(cId))) {
            int screenId = containmentGroups.group(cId).readEntry("lastScreen", IDNULL);

            if (isValid(screenId) && !screens.contains(screenId)) {
                screens << screenId;
            }
        }
    }

    return screens;
}

}
}
