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

#include "abstractlayout.h"

// Qt
#include <QDir>
#include <QDebug>
#include <QFile>

namespace Latte {
namespace Layout {

AbstractLayout::AbstractLayout(QObject *parent, QString layoutFile, QString assignedName)
    : QObject(parent)
{
    qDebug() << "Layout file to create object: " << layoutFile << " with name: " << assignedName;

    if (QFile(layoutFile).exists()) {
        if (assignedName.isEmpty()) {
            assignedName =  layoutName(layoutFile);
        }

        //!this order is important because setFile initializes also the m_layoutGroup
        setFile(layoutFile);
        setName(assignedName);
        loadConfig();
        init();

        m_loadedCorrectly = true;
    }
}

AbstractLayout::~AbstractLayout()
{
}

void AbstractLayout::init()
{
    connect(this, &AbstractLayout::backgroundChanged, this, &AbstractLayout::saveConfig);
    connect(this, &AbstractLayout::versionChanged, this, &AbstractLayout::saveConfig);
    connect(this, &AbstractLayout::colorChanged, this, &AbstractLayout::textColorChanged);
//    connect(this, &ActiveLayout::disableBordersForMaximizedWindowsChanged, this, &ActiveLayout::saveConfig);
//    connect(this, &ActiveLayout::showInMenuChanged, this, &ActiveLayout::saveConfig);
    connect(this, &AbstractLayout::textColorChanged, this, &AbstractLayout::saveConfig);
//    connect(this, &ActiveLayout::launchersChanged, this, &ActiveLayout::saveConfig);
//    connect(this, &ActiveLayout::lastUsedActivityChanged, this, &ActiveLayout::saveConfig);
//    connect(this, &ActiveLayout::preferredForShortcutsTouchedChanged, this, &ActiveLayout::saveConfig);*/
}

int AbstractLayout::version() const
{
    return m_version;
}

void AbstractLayout::setVersion(int ver)
{
    if (m_version == ver) {
        return;
    }

    m_version = ver;

    emit versionChanged();
}

QString AbstractLayout::background() const
{
    return m_background;
}

void AbstractLayout::setBackground(QString path)
{
    if (path == m_background) {
        return;
    }

    if (!path.isEmpty() && !QFileInfo(path).exists()) {
        return;
    }

    m_background = path;

    //! initialize the text color also
    if (path.isEmpty()) {
        setTextColor(QString());
    }

    emit backgroundChanged();
}


QString AbstractLayout::file() const
{
    return m_layoutFile;
}

void AbstractLayout::setFile(QString file)
{
    if (m_layoutFile == file) {
        return;
    }

    qDebug() << "Layout file:" << file;

    m_layoutFile = file;

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(m_layoutFile);
    m_layoutGroup = KConfigGroup(filePtr, "LayoutSettings");

    emit fileChanged();
}

QString AbstractLayout::name() const
{
    return m_layoutName;
}

void AbstractLayout::setName(QString name)
{
    if (m_layoutName == name) {
        return;
    }

    qDebug() << "Layout name:" << name;

    m_layoutName = name;

    emit nameChanged();
}

QString AbstractLayout::color() const
{
    return m_color;
}

void AbstractLayout::setColor(QString color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;
    emit colorChanged();
}

QString AbstractLayout::textColor() const
{
    //! the user is in default layout theme
    if (m_background.isEmpty()) {
        if (m_color == "blue") {
            return "#D7E3FF";
        } else if (m_color == "brown") {
            return "#F1DECB";
        } else if (m_color == "darkgrey") {
            return "#ECECEC";
        } else if (m_color == "gold") {
            return "#7C3636";
        } else if (m_color == "green") {
            return "#4D7549";
        } else if (m_color == "lightskyblue") {
            return "#0C2A43";
        } else if (m_color == "orange") {
            return "#6F3902";
        } else if (m_color == "pink") {
            return "#743C46";
        } else if (m_color == "purple") {
            return "#ECD9FF";
        }  else if (m_color == "red") {
            return "#F3E4E4";
        }  else if (m_color == "wheat") {
            return "#6A4E25";
        }  else {
            return "#FCFCFC";
        }
    }

    return "#" + m_textColor;
}

void AbstractLayout::setTextColor(QString color)
{
    //! remove # if someone is trying to set it this way
    if (color.startsWith("#")) {
        color.remove(0, 1);
    }

    if (m_textColor == color) {
        return;
    }

    m_textColor = color;
    emit textColorChanged();
}


QString AbstractLayout::layoutName(const QString &fileName)
{
    int lastSlash = fileName.lastIndexOf("/");
    QString tempLayoutFile = fileName;
    QString layoutName = tempLayoutFile.remove(0, lastSlash + 1);

    int ext = layoutName.lastIndexOf(".layout.latte");
    layoutName = layoutName.remove(ext, 13);

    return layoutName;
}

void AbstractLayout::loadConfig()
{
    m_version = m_layoutGroup.readEntry("version", 2);
    m_color = m_layoutGroup.readEntry("color", QString("blue"));
  //  m_disableBordersForMaximizedWindows = m_layoutGroup.readEntry("disableBordersForMaximizedWindows", false);
  //  m_showInMenu = m_layoutGroup.readEntry("showInMenu", false);
    m_textColor = m_layoutGroup.readEntry("textColor", QString("fcfcfc"));
  //  m_activities = m_layoutGroup.readEntry("activities", QStringList());
  //  m_launchers = m_layoutGroup.readEntry("launchers", QStringList());
  //  m_lastUsedActivity = m_layoutGroup.readEntry("lastUsedActivity", QString());
  //  m_preferredForShortcutsTouched = m_layoutGroup.readEntry("preferredForShortcutsTouched", false);

    QString back = m_layoutGroup.readEntry("background", "");

    if (!back.isEmpty()) {
        if (QFileInfo(back).exists()) {
            m_background = back;
        } else {
            m_layoutGroup.writeEntry("background", QString());
        }
    }

  //  emit activitiesChanged();*/
}

void AbstractLayout::saveConfig()
{
    qDebug() << "abstract layout is saving... for layout:" << m_layoutName;
    m_layoutGroup.writeEntry("version", m_version);
//    m_layoutGroup.writeEntry("showInMenu", m_showInMenu);
    m_layoutGroup.writeEntry("color", m_color);
 //   m_layoutGroup.writeEntry("disableBordersForMaximizedWindows", m_disableBordersForMaximizedWindows);
 //   m_layoutGroup.writeEntry("launchers", m_launchers);
    m_layoutGroup.writeEntry("background", m_background);
 //   m_layoutGroup.writeEntry("activities", m_activities);
 //   m_layoutGroup.writeEntry("lastUsedActivity", m_lastUsedActivity);
    m_layoutGroup.writeEntry("textColor", m_textColor);
  //  m_layoutGroup.writeEntry("preferredForShortcutsTouched", m_preferredForShortcutsTouched);

    m_layoutGroup.sync();
}

}
}
