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

// KDE
#include <KSharedConfig>

namespace Latte {
namespace Layout {

const QString AbstractLayout::MultipleLayoutsName = ".multiple-layouts_hidden";

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
    connect(this, &AbstractLayout::backgroundStyleChanged, this, &AbstractLayout::backgroundChanged);
    connect(this, &AbstractLayout::backgroundStyleChanged, this, &AbstractLayout::textColorChanged);
    connect(this, &AbstractLayout::customBackgroundChanged, this, &AbstractLayout::backgroundChanged);
    connect(this, &AbstractLayout::customTextColorChanged, this, &AbstractLayout::textColorChanged);
    connect(this, &AbstractLayout::colorChanged, this, &AbstractLayout::backgroundChanged);
    connect(this, &AbstractLayout::colorChanged, this, &AbstractLayout::textColorChanged);

    connect(this, &AbstractLayout::customBackgroundChanged, this, &AbstractLayout::saveConfig);
    connect(this, &AbstractLayout::customTextColorChanged, this, &AbstractLayout::saveConfig);
    connect(this, &AbstractLayout::colorChanged, this, &AbstractLayout::saveConfig);

    connect(this, &AbstractLayout::lastUsedActivityChanged, this, &AbstractLayout::saveConfig);
    connect(this, &AbstractLayout::launchersChanged, this, &AbstractLayout::saveConfig);
    connect(this, &AbstractLayout::preferredForShortcutsTouchedChanged, this, &AbstractLayout::saveConfig);
    connect(this, &AbstractLayout::versionChanged, this, &AbstractLayout::saveConfig);
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


bool AbstractLayout::preferredForShortcutsTouched() const
{
    return m_preferredForShortcutsTouched;
}

void AbstractLayout::setPreferredForShortcutsTouched(bool touched)
{
    if (m_preferredForShortcutsTouched == touched) {
        return;
    }

    m_preferredForShortcutsTouched = touched;
    emit preferredForShortcutsTouchedChanged();
}

QString AbstractLayout::background() const
{
    if (m_backgroundStyle == Types::ColorStyle) {
        return m_color;
    } else {
        return m_customBackground;
    }
}

QString AbstractLayout::textColor() const
{
    if (m_backgroundStyle == Types::ColorStyle) {
        return predefinedTextColor();
    } else {
        return m_customTextColor;
    }
}

Types::BackgroundStyle AbstractLayout::backgroundStyle() const
{
    return m_backgroundStyle;
}

void AbstractLayout::setBackgroundStyle(const Types::BackgroundStyle &style)
{
    if (m_backgroundStyle == style) {
        return;
    }

    m_backgroundStyle = style;
    emit backgroundStyleChanged();
}


QString AbstractLayout::customBackground() const
{
    return m_customBackground;
}

void AbstractLayout::setCustomBackground(const QString &background)
{
    if (m_customBackground == background) {
        return;
    }

    m_customBackground = background;

    emit customBackgroundChanged();
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

QString AbstractLayout::lastUsedActivity()
{
    return m_lastUsedActivity;
}

void AbstractLayout::clearLastUsedActivity()
{
    m_lastUsedActivity = "";
    emit lastUsedActivityChanged();
}

QString AbstractLayout::predefinedTextColor() const
{
    //! the user is in default layout theme

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

QString AbstractLayout::customTextColor() const
{
    return m_customTextColor;
}

void AbstractLayout::setCustomTextColor(const QString &customColor)
{
    QString cuColor = customColor;

    //! remove # if someone is trying to set it this way
    if (cuColor.startsWith("#")) {
        cuColor.remove(0, 1);
    }

    if (m_customTextColor == cuColor) {
        return;
    }

    m_customTextColor = cuColor;
    emit customTextColorChanged();
}

QStringList AbstractLayout::launchers() const
{
    return m_launchers;
}

void AbstractLayout::setLaunchers(QStringList launcherList)
{
    if (m_launchers == launcherList)
        return;

    m_launchers = launcherList;

    emit launchersChanged();
}

Type AbstractLayout::type() const
{
    return Type::Abstract;
}

QList<Plasma::Types::Location> combinedFreeEdges(const QList<Plasma::Types::Location> &edges1, const QList<Plasma::Types::Location> &edges2)
{
    QList<Plasma::Types::Location> validFreeEdges;

    for (int i=0; i<edges1.count(); ++i) {
        if (edges2.contains(edges1[i])) {
            validFreeEdges << edges1[i];
        }
    }

    return validFreeEdges;
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
    m_launchers = m_layoutGroup.readEntry("launchers", QStringList());
    m_lastUsedActivity = m_layoutGroup.readEntry("lastUsedActivity", QString());
    m_preferredForShortcutsTouched = m_layoutGroup.readEntry("preferredForShortcutsTouched", false);

    m_color = m_layoutGroup.readEntry("color", QString("blue"));
    m_backgroundStyle = static_cast<Types::BackgroundStyle>(m_layoutGroup.readEntry("backgroundStyle", (int)Types::ColorStyle));

    QString deprecatedTextColor = m_layoutGroup.readEntry("textColor", QString("fcfcfc"));
    QString deprecatedBackground = m_layoutGroup.readEntry("background", QString());

    if (deprecatedBackground.startsWith("/")) {
        m_customBackground = deprecatedBackground;
        m_customTextColor = deprecatedTextColor;
        setBackgroundStyle(Types::CustomBackgroundStyle);

        m_layoutGroup.writeEntry("background", QString());
        m_layoutGroup.writeEntry("textColor", QString());

        saveConfig();
    } else {
        m_customBackground = m_layoutGroup.readEntry("customBackground", QString(""));
        m_customTextColor = m_layoutGroup.readEntry("customTextColor", QString("fcfcfc"));
    }
}

void AbstractLayout::saveConfig()
{
    qDebug() << "abstract layout is saving... for layout:" << m_layoutName;
    m_layoutGroup.writeEntry("version", m_version);
    m_layoutGroup.writeEntry("color", m_color);
    m_layoutGroup.writeEntry("launchers", m_launchers);
    m_layoutGroup.writeEntry("backgroundStyle", (int)m_backgroundStyle);
    m_layoutGroup.writeEntry("customBackground", m_customBackground);
    m_layoutGroup.writeEntry("customTextColor", m_customTextColor);
    m_layoutGroup.writeEntry("lastUsedActivity", m_lastUsedActivity);
    m_layoutGroup.writeEntry("preferredForShortcutsTouched", m_preferredForShortcutsTouched);

    m_layoutGroup.sync();
}

}
}
