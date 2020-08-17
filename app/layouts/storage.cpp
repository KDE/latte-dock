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
#include "importer.h"
#include "manager.h"
#include "../lattecorona.h"
#include "../layout/abstractlayout.h"
#include "../layout/storage.h"

// Qt
#include <QDebug>
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
namespace Layouts {

Storage::Storage()
{
    qDebug() << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> LAYOUTS::STORAGE, TEMP DIR ::: " << m_storageTmpDir.path();
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
    QString temp2File = newUniqueIdsLayoutFromFile(layout, temp1FilePath);


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

QString Storage::newUniqueIdsLayoutFromFile(const Layout::GenericLayout *layout, QString file)
{
    if (!layout->corona()) {
        return QString();
    }

    QString tempFile = m_storageTmpDir.path() + "/" + layout->name() + ".views.newids";

    QFile copyFile(tempFile);

    if (copyFile.exists()) {
        copyFile.remove();
    }

    //! BEGIN updating the ids in the temp file
    QStringList allIds;
    allIds << layout->corona()->containmentsIds();
    allIds << layout->corona()->appletsIds();

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

        if (layout->corona()->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
            investigate_conts.group(cId).writeEntry("layoutId", layout->name());
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

QList<Plasma::Containment *> Storage::importLayoutFile(const Layout::GenericLayout *layout, QString file)
{
    KSharedConfigPtr filePtr = KSharedConfig::openConfig(file);
    auto newContainments = layout->corona()->importLayout(KConfigGroup(filePtr, ""));

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
}
}
