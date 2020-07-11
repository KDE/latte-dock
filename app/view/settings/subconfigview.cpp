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

#include "subconfigview.h"

//local
#include <config-latte.h>
#include "../view.h"
#include "../../lattecorona.h"
#include "../../layouts/manager.h"
#include "../../plasma/extended/theme.h"
#include "../../settings/universalsettings.h"
#include "../../shortcuts/globalshortcuts.h"
#include "../../shortcuts/shortcutstracker.h"
#include "../../wm/abstractwindowinterface.h"

// KDE
#include <KLocalizedContext>
#include <KDeclarative/KDeclarative>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem>

namespace Latte {
namespace ViewPart {

SubConfigView::SubConfigView(Latte::View *view, const QString &title, const bool &isNormalWindow)
    : QQuickView(nullptr),
      m_isNormalWindow(isNormalWindow)
{
    m_corona = qobject_cast<Latte::Corona *>(view->containment()->corona());

    setupWaylandIntegration();

    m_validTitle = title;
    setTitle(m_validTitle);

    if (KWindowSystem::isPlatformX11()) {
        m_corona->wm()->registerIgnoredWindow(winId());
    } else {
        connect(m_corona->wm(), &WindowSystem::AbstractWindowInterface::latteWindowAdded, this, &SubConfigView::updateWaylandId);
    }

    setScreen(view->screen());
    setIcon(qGuiApp->windowIcon());

    if (!m_isNormalWindow) {
        m_corona->wm()->setViewExtraFlags(this, true, Latte::Types::AlwaysVisible);
    }

    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(100);

    connections << connect(&m_screenSyncTimer, &QTimer::timeout, this, [this]() {
        if (!m_latteView) {
            return;
        }

        setScreen(m_latteView->screen());

        if (KWindowSystem::isPlatformX11()) {
            if (m_isNormalWindow) {
                setFlags(wFlags());
                m_corona->wm()->setViewExtraFlags(this, false, Latte::Types::NormalWindow);
            }
        }

        syncGeometry();
        syncSlideEffect();
    });
}

SubConfigView::~SubConfigView()
{
    qDebug() << validTitle() << " deleting...";

    m_corona->dialogShadows()->removeWindow(this);

    m_corona->wm()->unregisterIgnoredWindow(KWindowSystem::isPlatformX11() ? winId() : m_waylandWindowId);

    for (const auto &var : connections) {
        QObject::disconnect(var);
    }

    for (const auto &var : viewconnections) {
        QObject::disconnect(var);
    }
}

void SubConfigView::init()
{
    qDebug() << validTitle() << " : initialization started...";

    setDefaultAlphaBuffer(true);
    setColor(Qt::transparent);
    m_corona->dialogShadows()->addWindow(this);

    rootContext()->setContextProperty(QStringLiteral("viewConfig"), this);
    rootContext()->setContextProperty(QStringLiteral("shortcutsEngine"), m_corona->globalShortcuts()->shortcutsTracker());

    if (m_corona) {
        rootContext()->setContextProperty(QStringLiteral("universalSettings"), m_corona->universalSettings());
        rootContext()->setContextProperty(QStringLiteral("layoutsManager"), m_corona->layoutsManager());
        rootContext()->setContextProperty(QStringLiteral("themeExtended"), m_corona->themeExtended());
    }

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.setTranslationDomain(QStringLiteral("latte-dock"));
#if KF5_VERSION_MINOR >= 45
    kdeclarative.setupContext();
    kdeclarative.setupEngine(engine());
#else
    kdeclarative.setupBindings();
#endif
}

Qt::WindowFlags SubConfigView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint) & ~Qt::WindowDoesNotAcceptFocus;
}

QString SubConfigView::validTitle() const
{
    return m_validTitle;
}

Latte::View *SubConfigView::parentView() const
{
    return m_latteView;
}

void SubConfigView::setParentView(Latte::View *view)
{
    if (m_latteView == view) {
        return;
    }

    initParentView(view);
}

void SubConfigView::initParentView(Latte::View *view)
{
    for (const auto &var : viewconnections) {
        QObject::disconnect(var);
    }

    m_latteView = view;

    viewconnections << connect(m_latteView->positioner(), &ViewPart::Positioner::canvasGeometryChanged, this, &SubConfigView::syncGeometry);

    //! Assign app interfaces in be accessible through containment graphic item
    QQuickItem *containmentGraphicItem = qobject_cast<QQuickItem *>(m_latteView->containment()->property("_plasma_graphicObject").value<QObject *>());
    rootContext()->setContextProperty(QStringLiteral("plasmoid"), containmentGraphicItem);
    rootContext()->setContextProperty(QStringLiteral("latteView"), m_latteView);
}

void SubConfigView::requestActivate()
{
    if (KWindowSystem::isPlatformWayland() && m_shellSurface) {
        updateWaylandId();
        m_corona->wm()->requestActivate(m_waylandWindowId);
    } else {
        QQuickView::requestActivate();
    }
}

void SubConfigView::syncSlideEffect()
{
    if (!m_latteView || !m_latteView->containment()) {
        return;
    }

    auto slideLocation = WindowSystem::AbstractWindowInterface::Slide::None;

    switch (m_latteView->containment()->location()) {
    case Plasma::Types::TopEdge:
        slideLocation = WindowSystem::AbstractWindowInterface::Slide::Top;
        break;

    case Plasma::Types::RightEdge:
        slideLocation = WindowSystem::AbstractWindowInterface::Slide::Right;
        break;

    case Plasma::Types::BottomEdge:
        slideLocation = WindowSystem::AbstractWindowInterface::Slide::Bottom;
        break;

    case Plasma::Types::LeftEdge:
        slideLocation = WindowSystem::AbstractWindowInterface::Slide::Left;
        break;

    default:
        qDebug() << staticMetaObject.className() << "wrong location";
        break;
    }

    m_corona->wm()->slideWindow(*this, slideLocation);
}

void SubConfigView::setupWaylandIntegration()
{
    if (m_shellSurface || !KWindowSystem::isPlatformWayland() || !m_latteView || !m_latteView->containment()) {
        // already setup
        return;
    }

    if (m_corona) {
        using namespace KWayland::Client;
        PlasmaShell *interface = m_corona->waylandCoronaInterface();

        if (!interface) {
            return;
        }

        Surface *s = Surface::fromWindow(this);

        if (!s) {
            return;
        }

        qDebug() << "wayland primary settings surface was created...";

        m_shellSurface = interface->createSurface(s, this);

        if (m_isNormalWindow) {
            m_corona->wm()->setViewExtraFlags(m_shellSurface, false);
        } else {
            m_corona->wm()->setViewExtraFlags(m_shellSurface, true, Latte::Types::AlwaysVisible);
        }

        syncGeometry();
    }
}

bool SubConfigView::event(QEvent *e)
{
    if (e->type() == QEvent::PlatformSurface) {
        if (auto pe = dynamic_cast<QPlatformSurfaceEvent *>(e)) {
            switch (pe->surfaceEventType()) {
            case QPlatformSurfaceEvent::SurfaceCreated:

                if (m_shellSurface) {
                    break;
                }

                setupWaylandIntegration();
                break;

            case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed:
                if (m_shellSurface) {
                    delete m_shellSurface;
                    m_shellSurface = nullptr;
                    qDebug() << "WAYLAND config window surface was deleted...";
                }

                break;
            }
        }
    }

    return QQuickView::event(e);
}

void SubConfigView::updateWaylandId()
{
    Latte::WindowSystem::WindowId newId = m_corona->wm()->winIdFor("latte-dock", validTitle());

    if (m_waylandWindowId != newId) {
        if (!m_waylandWindowId.isNull()) {
            m_corona->wm()->unregisterIgnoredWindow(m_waylandWindowId);
        }

        m_waylandWindowId = newId;
        m_corona->wm()->registerIgnoredWindow(m_waylandWindowId);
    }
}

Plasma::FrameSvg::EnabledBorders SubConfigView::enabledBorders() const
{
    return m_enabledBorders;
}

}
}
