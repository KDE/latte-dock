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

#include "storage.h"

// local
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layouts/manager.h"
#include "../layouts/importer.h"
#include "../view/view.h"
// Qt
#include <QDir>
#include <QFile>
#include <QFileInfo>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

// Plasma
#include <Plasma>
#include <Plasma/Applet>
#include <Plasma/Containment>

namespace Latte {
namespace Layout {

Storage::Storage(GenericLayout *parent)
    : QObject(parent),
      m_layout(parent)
{
}

Storage::~Storage()
{
}

bool Storage::isWritable() const
{
    QFileInfo layoutFileInfo(m_layout->file());

    if (layoutFileInfo.exists() && !layoutFileInfo.isWritable()) {
        return false;
    } else {
        return true;
    }
}

bool Storage::isLatteContainment(Plasma::Containment *containment) const
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

void Storage::lock()
{
    QFileInfo layoutFileInfo(m_layout->file());

    if (layoutFileInfo.exists() && layoutFileInfo.isWritable()) {
        QFile(m_layout->file()).setPermissions(QFileDevice::ReadUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }
}

void Storage::unlock()
{
    QFileInfo layoutFileInfo(m_layout->file());

    if (layoutFileInfo.exists() && !layoutFileInfo.isWritable()) {
        QFile(m_layout->file()).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }
}

void Storage::importToCorona()
{
    if (!m_layout->corona()) {
        return;
    }

    //! Setting mutable for create a containment
    m_layout->corona()->setImmutability(Plasma::Types::Mutable);

    QString temp1FilePath = QDir::homePath() + "/.config/lattedock.copy1.bak";
    //! we need to copy first the layout file because the kde cache
    //! may not have yet been updated (KSharedConfigPtr)
    //! this way we make sure at the latest changes stored in the layout file
    //! will be also available when changing to Multiple Layouts
    QString tempLayoutFilePath = QDir::homePath() + "/.config/lattedock.layout.bak";

    //! WE NEED A WAY TO COPY A CONTAINMENT!!!!
    QFile tempLayoutFile(tempLayoutFilePath);
    QFile copyFile(temp1FilePath);
    QFile layoutOriginalFile(m_layout->file());

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
    QString temp2File = newUniqueIdsLayoutFromFile(temp1FilePath);


    //! Finally import the configuration
    importLayoutFile(temp2File);
}

void Storage::syncToLayoutFile(bool removeLayoutId)
{
    if (!m_layout->corona() || !isWritable()) {
        return;
    }

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(m_layout->file());

    KConfigGroup oldContainments = KConfigGroup(filePtr, "Containments");
    oldContainments.deleteGroup();
    oldContainments.sync();

    qDebug() << " LAYOUT :: " << m_layout->name() << " is syncing its original file.";

    for (const auto containment : *m_layout->containments()) {
        if (removeLayoutId) {
            containment->config().writeEntry("layoutId", "");
        }

        KConfigGroup newGroup = oldContainments.group(QString::number(containment->id()));
        containment->config().copyTo(&newGroup);

        if (!removeLayoutId) {
            newGroup.writeEntry("layoutId", "");
            newGroup.sync();
        }
    }

    oldContainments.sync();
}

void Storage::copyView(Plasma::Containment *containment)
{
    if (!containment || !m_layout->corona())
        return;

    qDebug() << "copying containment layout";
    //! Setting mutable for create a containment
    m_layout->corona()->setImmutability(Plasma::Types::Mutable);

    QString temp1File = QDir::homePath() + "/.config/lattedock.copy1.bak";

    //! WE NEED A WAY TO COPY A CONTAINMENT!!!!
    QFile copyFile(temp1File);

    if (copyFile.exists())
        copyFile.remove();

    KSharedConfigPtr newFile = KSharedConfig::openConfig(temp1File);
    KConfigGroup copied_conts = KConfigGroup(newFile, "Containments");
    KConfigGroup copied_c1 = KConfigGroup(&copied_conts, QString::number(containment->id()));

    containment->config().copyTo(&copied_c1);

    //!investigate if there multiple systray(s) in the containment to copy also

    //! systrayId, systrayAppletId
    QHash<uint, QString> systraysInfo;
    auto applets = containment->config().group("Applets");

    for (const auto &applet : applets.groupList()) {
        KConfigGroup appletSettings = applets.group(applet).group("Configuration");

        int tSysId = appletSettings.readEntry("SystrayContainmentId", -1);

        if (tSysId != -1) {
            systraysInfo[tSysId] = applet;
            qDebug() << "systray with id "<< tSysId << " was found in the containment... ::: " << tSysId;
        }
    }

    if (systraysInfo.count() > 0) {
        for(const auto systrayId : systraysInfo.keys()) {
            Plasma::Containment *systray{nullptr};

            for (const auto containment : m_layout->corona()->containments()) {
                if (containment->id() == systrayId) {
                    systray = containment;
                    break;
                }
            }

            if (systray) {
                KConfigGroup copied_systray = KConfigGroup(&copied_conts, QString::number(systray->id()));
                systray->config().copyTo(&copied_systray);
            }
        }
    }
    //! end of systray specific code

    //! update ids to unique ones
    QString temp2File = newUniqueIdsLayoutFromFile(temp1File);


    //! Don't create LatteView when the containment is created because we must update
    //! its screen settings first
    m_layout->setBlockAutomaticLatteViewCreation(true);
    //! Finally import the configuration
    QList<Plasma::Containment *> importedDocks = importLayoutFile(temp2File);

    Plasma::Containment *newContainment{nullptr};

    if (importedDocks.size() == 1) {
        newContainment = importedDocks[0];
    }

    if (!newContainment || !newContainment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    auto config = newContainment->config();

    //in multi-screen environment the copied dock is moved to alternative screens first
    const auto screens = qGuiApp->screens();
    auto dock =  m_layout->viewForContainment(containment);

    bool setOnExplicitScreen = false;

    int dockScrId = -1;
    int copyScrId = -1;

    if (dock) {
        dockScrId = dock->positioner()->currentScreenId();
        qDebug() << "COPY DOCK SCREEN ::: " << dockScrId;

        if (dockScrId != -1 && screens.count() > 1) {
            for (const auto scr : screens) {
                copyScrId = m_layout->corona()->screenPool()->id(scr->name());

                //the screen must exist and not be the same with the original dock
                if (copyScrId > -1 && copyScrId != dockScrId) {
                    QList<Plasma::Types::Location> fEdges = m_layout->freeEdges(copyScrId);

                    if (fEdges.contains((Plasma::Types::Location)containment->location())) {
                        ///set this containment to an explicit screen
                        config.writeEntry("onPrimary", false);
                        config.writeEntry("lastScreen", copyScrId);
                        newContainment->setLocation(containment->location());

                        qDebug() << "COPY DOCK SCREEN NEW SCREEN ::: " << copyScrId;

                        setOnExplicitScreen = true;
                        break;
                    }
                }
            }
        }
    }

    if (!setOnExplicitScreen) {
        QList<Plasma::Types::Location> edges = m_layout->freeEdges(newContainment->screen());

        if (edges.count() > 0) {
            newContainment->setLocation(edges.at(0));
        } else {
            newContainment->setLocation(Plasma::Types::BottomEdge);
        }

        config.writeEntry("onPrimary", true);
        config.writeEntry("lastScreen", dockScrId);
    }

    newContainment->config().sync();

    if (setOnExplicitScreen && copyScrId > -1) {
        qDebug() << "Copy Dock in explicit screen ::: " << copyScrId;
        m_layout->addView(newContainment, false, copyScrId);
        newContainment->reactToScreenChange();
    } else {
        qDebug() << "Copy Dock in current screen...";
        m_layout->addView(newContainment, false, dockScrId);
    }

    m_layout->setBlockAutomaticLatteViewCreation(false);
}

QList<Plasma::Containment *> Storage::importLayoutFile(QString file)
{
    KSharedConfigPtr filePtr = KSharedConfig::openConfig(file);
    auto newContainments = m_layout->corona()->importLayout(KConfigGroup(filePtr, ""));

    ///Find latte and systray containments
    qDebug() << " imported containments ::: " << newContainments.length();

    QList<Plasma::Containment *> importedDocks;
    //QList<Plasma::Containment *> systrays;

    for (const auto containment : newContainments) {
        if (isLatteContainment(containment)) {
            qDebug() << "new latte containment id: " << containment->id();
            importedDocks << containment;
        }
    }

    return importedDocks;
}

void Storage::systraysInformation(QHash<int, QList<int>> &systrays, QList<int> &assignedSystrays, QList<int> &orphanSystrays)
{
    systrays.clear();
    assignedSystrays.clear();
    orphanSystrays.clear();

    KSharedConfigPtr lFile = KSharedConfig::openConfig(m_layout->file());
    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    //! assigned systrays
    for (const auto &cId : containmentGroups.groupList()) {
        if (isLatteContainment(containmentGroups.group(cId))) {
            auto applets = containmentGroups.group(cId).group("Applets");

            for (const auto &applet : applets.groupList()) {
                KConfigGroup appletSettings = applets.group(applet).group("Configuration");
                int tSysId = appletSettings.readEntry("SystrayContainmentId", -1);

                if (tSysId != -1) {
                    assignedSystrays << tSysId;
                    systrays[cId.toInt()].append(tSysId);
                }
            }
        }
    }

    //! orphan systrays
    for (const auto &cId : containmentGroups.groupList()) {
        if (!isLatteContainment(containmentGroups.group(cId)) && !assignedSystrays.contains(cId.toInt())) {
            orphanSystrays << cId.toInt();
        }
    }
}

QList<ViewData> Storage::viewsData(const QHash<int, QList<int>> &systrays)
{
    QList<ViewData> viewsData;

    KSharedConfigPtr lFile = KSharedConfig::openConfig(m_layout->file());
    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    for (const auto &cId : containmentGroups.groupList()) {
        if (isLatteContainment(containmentGroups.group(cId))) {
            ViewData vData;
            int id = cId.toInt();

            //! id
            vData.id = id;

            //! active
            vData.active = false;

            //! onPrimary
            vData.onPrimary = containmentGroups.group(cId).readEntry("onPrimary", true);

            //! Screen
            vData.screenId = containmentGroups.group(cId).readEntry("lastScreen", -1);

            //! location
            vData.location = containmentGroups.group(cId).readEntry("location", (int)Plasma::Types::BottomEdge);

            //! systrays
            vData.systrays = systrays[id];

            viewsData << vData;
        }
    }

    return viewsData;
}

QList<int> Storage::viewsScreens()
{
    QList<int> screens;

    KSharedConfigPtr lFile = KSharedConfig::openConfig(m_layout->file());

    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    for (const auto &cId : containmentGroups.groupList()) {
        if (isLatteContainment(containmentGroups.group(cId))) {
            int screenId = containmentGroups.group(cId).readEntry("lastScreen", -1);

            if (screenId != -1 && !screens.contains(screenId)) {
                screens << screenId;
            }
        }
    }

    return screens;
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

QString Storage::newUniqueIdsLayoutFromFile(QString file)
{
    if (!m_layout->corona()) {
        return QString();
    }

    QString tempFile = QDir::homePath() + "/.config/lattedock.copy2.bak";

    QFile copyFile(tempFile);

    if (copyFile.exists()) {
        copyFile.remove();
    }

    //! BEGIN updating the ids in the temp file
    QStringList allIds;
    allIds << m_layout->corona()->containmentsIds();
    allIds << m_layout->corona()->appletsIds();

    QStringList toInvestigateContainmentIds;
    QStringList toInvestigateAppletIds;
    QStringList toInvestigateSystrayContIds;

    //! first is the systray containment id
    QHash<QString, QString> systrayParentContainmentIds;
    QHash<QString, QString> systrayAppletIds;

    //qDebug() << "Ids:" << allIds;

    //qDebug() << "to copy containments: " << toCopyContainmentIds;
    //qDebug() << "to copy applets: " << toCopyAppletIds;

    QStringList assignedIds;
    QHash<QString, QString> assigned;

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(file);
    KConfigGroup investigate_conts = KConfigGroup(filePtr, "Containments");

    //! Record the containment and applet ids
    for (const auto &cId : investigate_conts.groupList()) {
        toInvestigateContainmentIds << cId;
        auto appletsEntries = investigate_conts.group(cId).group("Applets");
        toInvestigateAppletIds << appletsEntries.groupList();

        //! investigate for systrays
        for (const auto &appletId : appletsEntries.groupList()) {
            KConfigGroup appletSettings = appletsEntries.group(appletId).group("Configuration");

            int tSysId = appletSettings.readEntry("SystrayContainmentId", -1);

            //! It is a systray !!!
            if (tSysId != -1) {
                QString tSysIdStr = QString::number(tSysId);
                toInvestigateSystrayContIds << tSysIdStr;
                systrayParentContainmentIds[tSysIdStr] = cId;
                systrayAppletIds[tSysIdStr] = appletId;
                qDebug() << "systray was found in the containment...";
            }
        }
    }

    //! Reassign containment and applet ids to unique ones
    for (const auto &contId : toInvestigateContainmentIds) {
        QString newId = availableId(allIds, assignedIds, 12);

        assignedIds << newId;
        assigned[contId] = newId;
    }

    for (const auto &appId : toInvestigateAppletIds) {
        QString newId = availableId(allIds, assignedIds, 40);

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

        if (m_layout->corona()->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
            investigate_conts.group(cId).writeEntry("layoutId", m_layout->name());
        }
    }

    //! must update also the systray id in its applet
    for (const auto &systrayId : toInvestigateSystrayContIds) {
        KConfigGroup systrayParentContainment = investigate_conts.group(systrayParentContainmentIds[systrayId]);
        systrayParentContainment.group("Applets").group(systrayAppletIds[systrayId]).group("Configuration").writeEntry("SystrayContainmentId", assigned[systrayId]);
        systrayParentContainment.sync();
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

bool Storage::appletGroupIsValid(KConfigGroup appletGroup)
{
    return !( appletGroup.keyList().count() == 0
              && appletGroup.groupList().count() == 1
              && appletGroup.groupList().at(0) == "Configuration"
              && appletGroup.group("Configuration").keyList().count() == 1
              && appletGroup.group("Configuration").hasKey("PreloadWeight") );
}

bool Storage::layoutIsBroken(QStringList &errors) const
{
    if (m_layout->file().isEmpty() || !QFile(m_layout->file()).exists()) {
        return false;
    }

    QStringList ids;
    QStringList conts;
    QStringList applets;

    KSharedConfigPtr lFile = KSharedConfig::openConfig(m_layout->file());

    if (!m_layout->corona()) {
        KConfigGroup containmentsEntries = KConfigGroup(lFile, "Containments");
        ids << containmentsEntries.groupList();
        conts << ids;

        for (const auto &cId : containmentsEntries.groupList()) {
            auto appletsEntries = containmentsEntries.group(cId).group("Applets");

            QStringList validAppletIds;
            bool updated{false};

            for (const auto &appletId : appletsEntries.groupList()) {
                KConfigGroup appletGroup = appletsEntries.group(appletId);

                if (appletGroupIsValid(appletGroup)) {
                    validAppletIds << appletId;
                } else {
                    updated = true;
                    //! heal layout file by removing applet config records that are not used any more
                    qDebug() << "Layout: " << m_layout->name() << " removing deprecated applet : " << appletId;
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
        for (const auto containment : *m_layout->containments()) {
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
        qDebug() << "   ----   ERROR - BROKEN LAYOUT :: " << m_layout->name() << " ----";

        if (!m_layout->corona()) {
            qDebug() << "   --- storaged file : " << m_layout->file();
        } else {
            if (m_layout->corona()->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
                qDebug() << "   --- in multiple layouts hidden file : " << Layouts::Importer::layoutFilePath(AbstractLayout::MultipleLayoutsName);
            } else {
                qDebug() << "   --- in active layout file : " << m_layout->file();
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

        if (!m_layout->corona()) {
            KConfigGroup containmentsEntries = KConfigGroup(lFile, "Containments");

            for (const auto &cId : containmentsEntries.groupList()) {
                auto appletsEntries = containmentsEntries.group(cId).group("Applets");

                qDebug() << " CONTAINMENT : " << cId << " APPLETS : " << appletsEntries.groupList();
            }
        } else {
            for (const auto containment : *m_layout->containments()) {
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


}
}
