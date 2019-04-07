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

// Qt
#include <QObject>

namespace Latte {
class Corona;
class TopLayout;
}

namespace Latte {

//! This class is responsible to hold the settings for a specific layout.
//! It also updates always the relevant layout configuration concerning
//! its general settings (no the containments)
class ActiveLayout : public Layout::GenericLayout
{
    Q_OBJECT

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

    QString topLayoutName() const;
    void setTopLayoutName(QString name);

    QStringList activities() const;
    void setActivities(QStringList activities);

    //! OVERRIDE GeneralLayout implementations
    void unloadContainments() override;
    const QStringList appliedActivities() override;
    QList<Latte::View *> latteViews() override;

    int viewsCount(int screen) const override;
    int viewsCount(QScreen *screen) const override;
    int viewsCount() const override;

    //! Available edges for specific view in that screen
    QList<Plasma::Types::Location> availableEdgesForView(QScreen *scr, Latte::View *forView) const override;
    //! All free edges in that screen
    QList<Plasma::Types::Location> freeEdges(QScreen *scr) const override;
    QList<Plasma::Types::Location> freeEdges(int screen) const override;

    QList<Latte::View *> sortedLatteViews(QList<Latte::View *> views = QList<Latte::View *>()) override;
    QList<Latte::View *> viewsWithPlasmaShortcuts() override;

signals:
    void activitiesChanged();
    void disableBordersForMaximizedWindowsChanged();
    void showInMenuChanged();
    void topLayoutNameChanged();

private slots:
    void loadConfig();
    void saveConfig();

    void setTopLayout(TopLayout *layout);

private:
    void init();
    void importLocalLayout(QString file);

    bool kwin_disabledMaximizedBorders() const;
    void kwin_setDisabledMaximizedBorders(bool disable);

private:
    bool m_disableBordersForMaximizedWindows{false};
    bool m_showInMenu{false};
    QString m_topLayoutName;
    QStringList m_activities;

    QPointer<TopLayout> m_topLayout;
};

}

#endif //ACTIVELAYOUT_H
