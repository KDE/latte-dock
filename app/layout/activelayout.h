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

#ifndef ACTIVELAYOUT_H
#define ACTIVELAYOUT_H

// local
#include "genericlayout.h"
#include "../../liblatte2/types.h"

// Qt
#include <QObject>
#include <QPointer>
#include <QQuickView>
#include <QScreen>

// Plasma
#include <Plasma>

namespace Plasma {
class Applet;
class Containment;
class Types;
}

namespace Latte {
class Corona;
class View;
}

namespace Latte {

//! This class is responsible to hold the settings for a specific layout.
//! It also updates always the relevant layout configuration concerning
//! its general settings (no the containments)
class ActiveLayout : public Layout::GenericLayout
{
    Q_OBJECT
  //  Q_PROPERTY(bool showInMenu READ showInMenu WRITE setShowInMenu NOTIFY showInMenuChanged)
   // Q_PROPERTY(QStringList activities READ activities WRITE setActivities NOTIFY activitiesChanged)

public:
    ActiveLayout(QObject *parent, QString layoutFile, QString layoutName = QString());
    ~ActiveLayout() override;

    void initToCorona(Latte::Corona *corona);

    bool disableBordersForMaximizedWindows() const;
    void setDisableBordersForMaximizedWindows(bool disable);

    bool showInMenu() const;
    void setShowInMenu(bool show);

    //!this layout is loaded and running
    bool isActiveLayout() const;
    //!it is original layout compared to pseudo-layouts that are combinations of multiple-original layouts
    bool isOriginalLayout() const;

    QStringList activities() const;
    void setActivities(QStringList activities);

    const QStringList appliedActivities() override;

signals:
    void activitiesChanged();
    void disableBordersForMaximizedWindowsChanged();
    void showInMenuChanged();

private slots:
    void loadConfig();
    void saveConfig();

private:
    void init();
    void importLocalLayout(QString file);
    void updateLastUsedActivity();

    bool kwin_disabledMaximizedBorders() const;
    void kwin_setDisabledMaximizedBorders(bool disable);

private:
    bool m_disableBordersForMaximizedWindows{false};
    bool m_showInMenu{false};
    QStringList m_activities;
};

}

#endif //ACTIVELAYOUT_H
