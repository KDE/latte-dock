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

#include "layoutsettings.h"

#include <QFile>
#include <KSharedConfig>

namespace Latte {

LayoutSettings::LayoutSettings(QObject *parent, QString layoutFile, QString assignedName)
    : QObject(parent)
{
    qDebug() << "Layout file to create object: " << layoutFile << " with name: " << assignedName;

    if (QFile(layoutFile).exists()) {
        if (assignedName.isEmpty()) {
            assignedName =  layoutName(layoutFile);
        }

        KSharedConfigPtr lConfig = KSharedConfig::openConfig(layoutFile);
        m_layoutGroup = KConfigGroup(lConfig, "LayoutSettings");

        setFile(layoutFile);
        setName(assignedName);
        loadConfig();
        init();
    }
}

LayoutSettings::~LayoutSettings()
{
    if (!m_layoutFile.isEmpty()) {
        //saveConfig();
        m_layoutGroup.sync();
    }
}

void LayoutSettings::init()
{
    connect(this, &LayoutSettings::activitiesChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::versionChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::colorChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::showInMenuChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::launchersChanged, this, &LayoutSettings::saveConfig);
}

int LayoutSettings::version() const
{
    return m_version;
}

void LayoutSettings::setVersion(int ver)
{
    if (m_version == ver) {
        return;
    }

    m_version = ver;

    emit versionChanged();
}

bool LayoutSettings::showInMenu() const
{
    return m_showInMenu;
}

void LayoutSettings::setShowInMenu(bool show)
{
    if (m_showInMenu == show) {
        return;
    }

    m_showInMenu = show;
    emit showInMenuChanged();
}

QString LayoutSettings::name() const
{
    return m_layoutName;
}

void LayoutSettings::setName(QString name)
{
    if (m_layoutName == name) {
        return;
    }

    qDebug() << "Layout name:" << name;

    m_layoutName = name;

    emit nameChanged();
}

QString LayoutSettings::color() const
{
    return m_color;
}

void LayoutSettings::setColor(QString color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;
    emit colorChanged();
}


QString LayoutSettings::file() const
{
    return m_layoutFile;
}

void LayoutSettings::setFile(QString file)
{
    if (m_layoutFile == file) {
        return;
    }

    qDebug() << "Layout file:" << file;

    m_layoutFile = file;
    emit fileChanged();
}

QStringList LayoutSettings::launchers() const
{
    return m_launchers;
}

void LayoutSettings::setLaunchers(QStringList launcherList)
{
    if (m_launchers == launcherList)
        return;

    m_launchers = launcherList;

    emit launchersChanged();
}

QStringList LayoutSettings::activities() const
{
    return m_activities;
}

void  LayoutSettings::setActivities(QStringList activities)
{
    if (m_activities == activities) {
        return;
    }

    m_activities = activities;

    emit activitiesChanged();
}

bool LayoutSettings::fileIsBroken() const
{
    if (m_layoutFile.isEmpty() || !QFile(m_layoutFile).exists()) {
        return false;
    }

    KSharedConfigPtr lFile = KSharedConfig::openConfig(m_layoutFile);
    KConfigGroup containmentsEntries = KConfigGroup(lFile, "Containments");

    QStringList ids;

    ids << containmentsEntries.groupList();
    QStringList conts;
    conts << ids;
    QStringList applets;

    foreach (auto cId, containmentsEntries.groupList()) {
        auto appletsEntries = containmentsEntries.group(cId).group("Applets");

        ids << appletsEntries.groupList();
        applets << appletsEntries.groupList();
    }

    QSet<QString> idsSet = QSet<QString>::fromList(ids);

    QMap<QString, int> countOfStrings;

    for (int i = 0; i < ids.count(); i++) {
        countOfStrings[ids[i]]++;
    }

    if (idsSet.count() != ids.count()) {
        qDebug() << "   ----   ERROR: BROKEN LAYOUT ----";

        foreach (QString c, conts) {
            if (applets.contains(c)) {
                qDebug() << "Error: Same applet and containment id found ::: " << c;
            }
        }

        for (int i = 0; i < ids.count(); ++i) {
            for (int j = i + 1; j < ids.count(); ++j) {
                if (ids[i] == ids[j]) {
                    qDebug() << "Error: Applets with same id ::: " << ids[i];
                }
            }
        }

        return true;
    }

    return false;
}


QString LayoutSettings::layoutName(const QString &fileName)
{
    int lastSlash = fileName.lastIndexOf("/");
    QString tempLayoutFile = fileName;
    QString layoutName = tempLayoutFile.remove(0, lastSlash + 1);

    int ext = layoutName.lastIndexOf(".layout.latte");
    layoutName = layoutName.remove(ext, 13);

    return layoutName;
}

void LayoutSettings::loadConfig()
{
    m_version = m_layoutGroup.readEntry("version", 2);
    m_color = m_layoutGroup.readEntry("color", QString("blue"));
    m_showInMenu = m_layoutGroup.readEntry("showInMenu", false);
    m_activities = m_layoutGroup.readEntry("activities", QStringList());
    m_launchers = m_layoutGroup.readEntry("launchers", QStringList());
}

void LayoutSettings::saveConfig()
{
    qDebug() << "layout is saving... for layout:" << m_layoutName;
    m_layoutGroup.writeEntry("version", m_version);
    m_layoutGroup.writeEntry("showInMenu", m_showInMenu);
    m_layoutGroup.writeEntry("color", m_color);
    m_layoutGroup.writeEntry("launchers", m_launchers);
    m_layoutGroup.writeEntry("activities", m_activities);

    m_layoutGroup.sync();
}

}
