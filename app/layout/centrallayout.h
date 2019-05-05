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

#ifndef CENTRALLAYOUT_H
#define CENTRALLAYOUT_H

// local
#include "genericlayout.h"

// Qt
#include <QObject>

namespace Latte {
class Corona;
class SharedLayout;
}

namespace Latte {

//! CentralLayout is a very IMPORTANT layout that is responsible for specific Activities or not
//! and it is used for all memory modes (SINGLE/MULTIPLE) at all times.
//!
//! It holds all the important settings in order to provide specific
//! behavior for the Activities is assigned at.
//! for example: activities for which its views should be shown,
//! if the maximized windows will be borderless,
//! if the layout will be shown at user layout contextmenu,
//! which shared layout will be used on top of that layout.
//!

class CentralLayout : public Layout::GenericLayout
{
    Q_OBJECT

public:
    CentralLayout(QObject *parent, QString layoutFile, QString layoutName = QString());
    ~CentralLayout() override;

    void initToCorona(Latte::Corona *corona);

    bool disableBordersForMaximizedWindows() const;
    void setDisableBordersForMaximizedWindows(bool disable);

    bool showInMenu() const;
    void setShowInMenu(bool show);

    QString sharedLayoutName() const;
    void setSharedLayoutName(QString name);

    QStringList activities() const;
    void setActivities(QStringList activities);

    SharedLayout *sharedLayout() const;
    void setSharedLayout(SharedLayout *layout);

    //! OVERRIDE GeneralLayout implementations
    void addView(Plasma::Containment *containment, bool forceOnPrimary = false, int explicitScreen = -1, Layout::ViewsMap *occupied = nullptr);
    void syncLatteViewsToScreens(Layout::ViewsMap *occupiedMap = nullptr) override;
    void unloadContainments() override;
    const QStringList appliedActivities() override;
    QList<Latte::View *> latteViews() override;

    int viewsCount(int screen) const override;
    int viewsCount(QScreen *screen) const override;
    int viewsCount() const override;

    Layout::Type type() const override;

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
    void sharedLayoutNameChanged();

private slots:
    void disconnectSharedConnections();

    void loadConfig();
    void saveConfig();

private:
    void init();
    void importLocalLayout(QString file);

    bool kwin_disabledMaximizedBorders() const;
    void kwin_setDisabledMaximizedBorders(bool disable);

private:
    bool m_disableBordersForMaximizedWindows{false};
    bool m_showInMenu{false};
    QString m_sharedLayoutName;
    QStringList m_activities;

    QPointer<SharedLayout> m_sharedLayout;

    QList<QMetaObject::Connection> m_sharedConnections;
};

}

#endif //CENTRALLAYOUT_H
