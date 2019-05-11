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

#include "centrallayout.h"

// local
#include "sharedlayout.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layouts/importer.h"
#include "../layouts/manager.h"
#include "../layouts/synchronizer.h"
#include "../settings/universalsettings.h"
#include "../view/view.h"
#include "../../liblatte2/types.h"

// Qt
#include <QtDBus/QtDBus>

// KDE
#include <KConfigGroup>
#include <KActivities/Consumer>

namespace Latte {

CentralLayout::CentralLayout(QObject *parent, QString layoutFile, QString assignedName)
    : Layout::GenericLayout(parent, layoutFile, assignedName)
{
    if (m_loadedCorrectly) {
        loadConfig();
        init();
    }
}

CentralLayout::~CentralLayout()
{
    if (!m_layoutFile.isEmpty()) {
        m_layoutGroup.sync();
    }
}

void CentralLayout::unloadContainments()
{
    Layout::GenericLayout::unloadContainments();

    if (m_sharedLayout) {
        disconnectSharedConnections();
        m_sharedLayout->removeCentralLayout(this);
    }
}

void CentralLayout::init()
{
    connect(this, &CentralLayout::activitiesChanged, this, &CentralLayout::saveConfig);
    connect(this, &CentralLayout::disableBordersForMaximizedWindowsChanged, this, &CentralLayout::saveConfig);
    connect(this, &CentralLayout::sharedLayoutNameChanged, this, &CentralLayout::saveConfig);
    connect(this, &CentralLayout::showInMenuChanged, this, &CentralLayout::saveConfig);
}

void CentralLayout::initToCorona(Latte::Corona *corona)
{
    if (GenericLayout::initToCorona(corona)) {
        connect(m_corona->universalSettings(), &UniversalSettings::canDisableBordersChanged, this, [&]() {
            if (m_corona->universalSettings()->canDisableBorders()) {
                kwin_setDisabledMaximizedBorders(disableBordersForMaximizedWindows());
            } else {
                kwin_setDisabledMaximizedBorders(false);
            }
        });


        if (m_corona->layoutsManager()->memoryUsage() == Types::SingleLayout && m_corona->universalSettings()->canDisableBorders()) {
            kwin_setDisabledMaximizedBorders(disableBordersForMaximizedWindows());
        } else if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
            connect(m_corona->layoutsManager(), &Layouts::Manager::currentLayoutNameChanged, this, [&]() {
                if (m_corona->universalSettings()->canDisableBorders()
                    && m_corona->layoutsManager()->currentLayoutName() == name()) {
                    kwin_setDisabledMaximizedBorders(disableBordersForMaximizedWindows());
                }
            });
        }

        //! Request the SharedLayout in case there is one and Latte is functioning in MultipleLayouts mode
        if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts && !m_sharedLayoutName.isEmpty()) {
            if (m_corona->layoutsManager()->synchronizer()->registerAtSharedLayout(this, m_sharedLayoutName)) {
                setSharedLayout(m_corona->layoutsManager()->synchronizer()->sharedLayout(m_sharedLayoutName));
            }
        }
    }
}

bool CentralLayout::disableBordersForMaximizedWindows() const
{
    return m_disableBordersForMaximizedWindows;
}

void CentralLayout::setDisableBordersForMaximizedWindows(bool disable)
{
    if (m_disableBordersForMaximizedWindows == disable) {
        return;
    }

    m_disableBordersForMaximizedWindows = disable;
    kwin_setDisabledMaximizedBorders(disable);

    emit disableBordersForMaximizedWindowsChanged();
}

bool CentralLayout::kwin_disabledMaximizedBorders() const
{
    //! Identify Plasma Desktop version
    QProcess process;
    process.start("kreadconfig5 --file kwinrc --group Windows --key BorderlessMaximizedWindows");
    process.waitForFinished();
    QString output(process.readAllStandardOutput());

    output = output.remove("\n");

    return (output == "true");
}

void CentralLayout::kwin_setDisabledMaximizedBorders(bool disable)
{
    if (kwin_disabledMaximizedBorders() == disable) {
        return;
    }

    QString disableText = disable ? "true" : "false";

    QProcess process;
    QString commandStr = "kwriteconfig5 --file kwinrc --group Windows --key BorderlessMaximizedWindows --type bool " + disableText;
    process.start(commandStr);
    process.waitForFinished();

    QDBusInterface iface("org.kde.KWin", "/KWin", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("reconfigure");
    }

}

bool CentralLayout::showInMenu() const
{
    return m_showInMenu;
}

void CentralLayout::setShowInMenu(bool show)
{
    if (m_showInMenu == show) {
        return;
    }

    m_showInMenu = show;
    emit showInMenuChanged();
}

Layout::Type CentralLayout::type() const
{
    return Layout::Type::Central;
}

QStringList CentralLayout::activities() const
{
    return m_activities;
}

void CentralLayout::setActivities(QStringList activities)
{
    if (m_activities == activities) {
        return;
    }

    m_activities = activities;

    emit activitiesChanged();
}

QString CentralLayout::sharedLayoutName() const
{
    return m_sharedLayoutName;
}

void CentralLayout::setSharedLayoutName(QString name)
{
    if (m_sharedLayoutName == name || (!Layouts::Importer::layoutExists(name) && !name.isEmpty())) {
        return;
    }

    m_sharedLayoutName = name;
    emit sharedLayoutNameChanged();
}

SharedLayout *CentralLayout::sharedLayout() const
{
    return m_sharedLayout;
}

void CentralLayout::setSharedLayout(SharedLayout *layout)
{
    if (m_sharedLayout == layout) {
        return;
    }

    disconnectSharedConnections();

    m_sharedLayout = layout;

    if (layout) {
        setSharedLayoutName(m_sharedLayout->name());

        //! attach new signals
        m_sharedConnections << connect(m_sharedLayout, &Layout::GenericLayout::viewsCountChanged, this, &Layout::GenericLayout::viewsCountChanged);
        m_sharedConnections << connect(m_sharedLayout, &Layout::AbstractLayout::nameChanged, this, [this]() {
            setSharedLayoutName(m_sharedLayout->name());
        });
        m_sharedConnections << connect(m_sharedLayout, &Layout::GenericLayout::viewEdgeChanged, this, [this]() {
            syncLatteViewsToScreens();
        });
    } else {
        setSharedLayoutName(QString());
    }

    emit viewsCountChanged();
}

void CentralLayout::disconnectSharedConnections()
{
    //! drop old signals
    for (const auto &sc : m_sharedConnections) {
        QObject::disconnect(sc);
    }

    m_sharedConnections.clear();
}

void CentralLayout::loadConfig()
{
    m_disableBordersForMaximizedWindows = m_layoutGroup.readEntry("disableBordersForMaximizedWindows", false);
    m_showInMenu = m_layoutGroup.readEntry("showInMenu", false);     
    m_activities = m_layoutGroup.readEntry("activities", QStringList());

    QString sharedLayoutName = m_layoutGroup.readEntry("sharedLayout", QString());

    if (Layouts::Importer::layoutExists(sharedLayoutName)) {
        m_sharedLayoutName = sharedLayoutName;
    }

    emit activitiesChanged();
}

void CentralLayout::saveConfig()
{
    qDebug() << "CENTRAL layout is saving... for layout:" << m_layoutName;
    m_layoutGroup.writeEntry("showInMenu", m_showInMenu);
    m_layoutGroup.writeEntry("disableBordersForMaximizedWindows", m_disableBordersForMaximizedWindows);
    m_layoutGroup.writeEntry("sharedLayout", m_sharedLayoutName);
    m_layoutGroup.writeEntry("activities", m_activities);

    m_layoutGroup.sync();
}

//! OVERRIDES

void CentralLayout::addView(Plasma::Containment *containment, bool forceOnPrimary, int explicitScreen, Layout::ViewsMap *occupied)
{
    if (m_sharedLayout) {
        //! consider already occupied edges from SharedLayout
        Layout::ViewsMap ocMap = m_sharedLayout->validViewsMap();
        Layout::GenericLayout::addView(containment, forceOnPrimary, explicitScreen, &ocMap);
    } else {
        Layout::GenericLayout::addView(containment, forceOnPrimary, explicitScreen, occupied);
    }
}

const QStringList CentralLayout::appliedActivities()
{
    if (!m_corona) {
        return {};
    }

    if (m_corona->layoutsManager()->memoryUsage() == Types::SingleLayout) {
        return {"0"};
    } else if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
        if (m_activities.isEmpty()) {
            return m_corona->layoutsManager()->synchronizer()->orphanedActivities();
        } else {
            return m_activities;
        }
    } else {
        return {"0"};
    }
}

QList<Latte::View *> CentralLayout::latteViews()
{
    if (m_sharedLayout) {
        QList<Latte::View *> views = Layout::GenericLayout::latteViews();
        views << m_sharedLayout->latteViews();

        return views;
    }

    return Layout::GenericLayout::latteViews();
}

int CentralLayout::viewsCount(int screen) const
{
    if (!m_corona) {
        return 0;
    }

    int views = Layout::GenericLayout::viewsCount(screen);

    if (m_sharedLayout) {
        QScreen *scr = m_corona->screenPool()->screenForId(screen);

        for (const auto view : m_sharedLayout->latteViews()) {
            if (view && view->screen() == scr && !view->containment()->destroyed()) {
                ++views;
            }
        }
    }

    return views;
}

int CentralLayout::viewsCount(QScreen *screen) const
{
    if (!m_corona) {
        return 0;
    }

    int views = Layout::GenericLayout::viewsCount(screen);

    if (m_sharedLayout) {
        for (const auto view : m_sharedLayout->latteViews()) {
            if (view && view->screen() == screen && !view->containment()->destroyed()) {
                ++views;
            }
        }
    }

    return views;
}

int CentralLayout::viewsCount() const
{
    if (!m_corona) {
        return 0;
    }

    int views = Layout::GenericLayout::viewsCount();

    if (m_sharedLayout) {
        for (const auto view : m_sharedLayout->latteViews()) {
            if (view && view->containment() && !view->containment()->destroyed()) {
                ++views;
            }
        }
    }

    return views;
}

QList<Plasma::Types::Location> CentralLayout::availableEdgesForView(QScreen *scr, Latte::View *forView) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    edges = Layout::GenericLayout::availableEdgesForView(scr, forView);

    if (m_sharedLayout) {
        for (const auto view : m_sharedLayout->latteViews()) {
            //! make sure that availabe edges takes into account only views that should be excluded,
            //! this is why the forView should not be excluded
            if (view && view != forView && view->positioner()->currentScreenName() == scr->name()) {
                edges.removeOne(view->location());
            }
        }
    }

    return edges;
}

QList<Plasma::Types::Location> CentralLayout::freeEdges(QScreen *scr) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    edges = Layout::GenericLayout::freeEdges(scr);

    if (m_sharedLayout) {
        for (const auto view : m_sharedLayout->latteViews()) {
            if (view && view->positioner()->currentScreenName() == scr->name()) {
                edges.removeOne(view->location());
            }
        }
    }

    return edges;
}

QList<Plasma::Types::Location> CentralLayout::freeEdges(int screen) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    edges = Layout::GenericLayout::freeEdges(screen);
    QScreen *scr = m_corona->screenPool()->screenForId(screen);

    if (m_sharedLayout) {
        for (const auto view : m_sharedLayout->latteViews()) {
            if (view && scr && view->positioner()->currentScreenName() == scr->name()) {
                edges.removeOne(view->location());
            }
        }
    }

    return edges;
}

QList<Latte::View *> CentralLayout::sortedLatteViews(QList<Latte::View *> views)
{
    QList<Latte::View *> vws = latteViews();

    return Layout::GenericLayout::sortedLatteViews(vws);
}

QList<Latte::View *> CentralLayout::viewsWithPlasmaShortcuts()
{
    QList<Latte::View *> combined = Layout::GenericLayout::viewsWithPlasmaShortcuts();

    if (m_sharedLayout) {
        combined << m_sharedLayout->viewsWithPlasmaShortcuts();
    }

    return combined;
}

void CentralLayout::syncLatteViewsToScreens(Layout::ViewsMap *occupiedMap)
{
    if (m_sharedLayout) {
        Layout::ViewsMap map = m_sharedLayout->validViewsMap();
        Layout::GenericLayout::syncLatteViewsToScreens(&map);
    } else {
        Layout::GenericLayout::syncLatteViewsToScreens();
    }
}

}
