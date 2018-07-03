/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "docksecconfigview.h"

#include "dockconfigview.h"
#include "dockview.h"
#include "panelshadows_p.h"
#include "../abstractwindowinterface.h"
#include "../dockcorona.h"

#include <QFontMetrics>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

#include <KLocalizedContext>
#include <KDeclarative/KDeclarative>

#include <Plasma/Package>

#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>

#include <KWindowSystem>

namespace Latte {

DockSecConfigView::DockSecConfigView(DockView *dockView, QWindow *parent)
    : QQuickView(nullptr),
      m_parent(parent),
      m_dockView(dockView)
{
    m_corona = qobject_cast<DockCorona *>(m_dockView->containment()->corona());

    setupWaylandIntegration();

    setResizeMode(QQuickView::SizeViewToRootObject);
    //setScreen(m_dockView->screen());

    if (dockView && dockView->containment()) {
        setIcon(qGuiApp->windowIcon());
    }

    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(100);

    connections << connect(&m_screenSyncTimer, &QTimer::timeout, this, [this]() {
        // setScreen(m_dockView->screen());
        setFlags(wFlags());
        syncGeometry();
        syncSlideEffect();
    });
    connections << connect(dockView->visibility(), &VisibilityManager::modeChanged, this, &DockSecConfigView::syncGeometry);

    m_thicknessSyncTimer.setSingleShot(true);
    m_thicknessSyncTimer.setInterval(200);
    connections << connect(&m_thicknessSyncTimer, &QTimer::timeout, this, [this]() {
        syncGeometry();
    });

    connections << connect(dockView, &DockView::normalThicknessChanged, [&]() {
        m_thicknessSyncTimer.start();
    });

    /*auto *dockCorona = qobject_cast<DockCorona *>(m_dockView->corona());

    if (dockCorona && m_configType == PrimaryConfig) {
        connections << connect(dockCorona, SIGNAL(raiseDocksTemporaryChanged()), this, SIGNAL(raiseDocksTemporaryChanged()));
    }*/
}

DockSecConfigView::~DockSecConfigView()
{
    qDebug() << "SecDockConfigView deleting ...";

    foreach (auto var, connections) {
        QObject::disconnect(var);
    }

    if (m_shellSurface) {
        delete m_shellSurface;
        m_shellSurface = nullptr;
    }

}

void DockSecConfigView::init()
{
    qDebug() << "dock secondary config view : initialization started...";

    setDefaultAlphaBuffer(true);
    setColor(Qt::transparent);
    PanelShadows::self()->addWindow(this);
    rootContext()->setContextProperty(QStringLiteral("dock"), m_dockView);
    rootContext()->setContextProperty(QStringLiteral("dockConfig"), this);
    rootContext()->setContextProperty(QStringLiteral("plasmoid"), m_dockView->containment()->property("_plasma_graphicObject").value<QObject *>());

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.setTranslationDomain(QStringLiteral("latte-dock"));
    kdeclarative.setupBindings();

    QByteArray tempFilePath = "lattedocksecondaryconfigurationui";

    updateEnabledBorders();

    auto source = QUrl::fromLocalFile(m_dockView->containment()->corona()->kPackage().filePath(tempFilePath));
    setSource(source);
    syncGeometry();
    syncSlideEffect();

    m_parent->requestActivate();
    qDebug() << "dock secondary config view : initialization ended...";
}

inline Qt::WindowFlags DockSecConfigView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) & ~Qt::WindowDoesNotAcceptFocus;
}

void DockSecConfigView::syncGeometry()
{
    if (!m_dockView->managedLayout() || !m_dockView->containment() || !rootObject())
        return;

    const QSize size(rootObject()->width(), rootObject()->height());
    setMaximumSize(size);
    setMinimumSize(size);
    resize(size);

    const auto location = m_dockView->containment()->location();
    const auto sGeometry = screen()->geometry();

    int clearThickness = m_dockView->normalThickness() + m_dockView->fontPixelSize();

    int secondaryConfigSpacing = 2 * m_dockView->fontPixelSize();

    QPoint position{0, 0};

    switch (m_dockView->containment()->formFactor()) {
        case Plasma::Types::Horizontal: {
            if (location == Plasma::Types::TopEdge) {
                int yPos = m_dockView->y() + clearThickness;

                position = {m_dockView->x() + secondaryConfigSpacing, yPos};
            } else if (location == Plasma::Types::BottomEdge) {

                int yPos;
                yPos = sGeometry.y() + sGeometry.height() - clearThickness - size.height();

                position = {m_dockView->x() + m_dockView->width() - secondaryConfigSpacing - size.width(), yPos};
            }
        }
        break;

        case Plasma::Types::Vertical: {
            if (location == Plasma::Types::LeftEdge) {
                position = {sGeometry.x() + clearThickness
                            , m_dockView->y() + secondaryConfigSpacing
                           };
            } else if (location == Plasma::Types::RightEdge) {
                position = {sGeometry.x() + sGeometry.width() - clearThickness - size.width()
                            , m_dockView->y() + secondaryConfigSpacing
                           };
            }
        }
        break;

        default:
            qWarning() << "no sync geometry, wrong formFactor";
            break;
    }

    updateEnabledBorders();

    setPosition(position);

    if (m_shellSurface) {
        m_shellSurface->setPosition(position);
    }
}

void DockSecConfigView::syncSlideEffect()
{
    if (!m_dockView->containment())
        return;

    auto slideLocation = WindowSystem::Slide::None;

    switch (m_dockView->containment()->location()) {
        case Plasma::Types::TopEdge:
            slideLocation = WindowSystem::Slide::Top;
            break;

        case Plasma::Types::RightEdge:
            slideLocation = WindowSystem::Slide::Right;
            break;

        case Plasma::Types::BottomEdge:
            slideLocation = WindowSystem::Slide::Bottom;
            break;

        case Plasma::Types::LeftEdge:
            slideLocation = WindowSystem::Slide::Left;
            break;

        default:
            qDebug() << staticMetaObject.className() << "wrong location";
            break;
    }

    m_corona->wm()->slideWindow(*this, slideLocation);
}

void DockSecConfigView::showEvent(QShowEvent *ev)
{
    QQuickWindow::showEvent(ev);

    m_corona->wm()->setDockExtraFlags(*this);
    setFlags(wFlags());

    m_corona->wm()->enableBlurBehind(*this);

    syncGeometry();
    syncSlideEffect();

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &DockSecConfigView::syncGeometry);

    emit showSignal();
}

void DockSecConfigView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);

    const auto *focusWindow = qGuiApp->focusWindow();

    if (focusWindow && (focusWindow->flags().testFlag(Qt::Popup)
                        || focusWindow->flags().testFlag(Qt::ToolTip)))
        return;

    const auto parent = qobject_cast<DockConfigView *>(m_parent);

    if (parent && !parent->sticker() && !parent->isActive()) {
        parent->hideConfigWindow();
    }
}

void DockSecConfigView::setupWaylandIntegration()
{
    if (m_shellSurface || !KWindowSystem::isPlatformWayland() || !m_dockView || !m_dockView->containment()) {
        // already setup
        return;
    }

    if (DockCorona *c = qobject_cast<DockCorona *>(m_dockView->containment()->corona())) {
        using namespace KWayland::Client;
        PlasmaShell *interface = c->waylandDockCoronaInterface();

        if (!interface) {
            return;
        }

        Surface *s = Surface::fromWindow(this);

        if (!s) {
            return;
        }

        qDebug() << "wayland dock window surface was created...";

        m_shellSurface = interface->createSurface(s, this);
        m_shellSurface->setSkipTaskbar(true);

        syncGeometry();
    }
}

bool DockSecConfigView::event(QEvent *e)
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
                        qDebug() << "WAYLAND secondary config window surface was deleted...";
                        PanelShadows::self()->removeWindow(this);
                    }

                    break;
            }
        }
    }

    return QQuickView::event(e);
}

void DockSecConfigView::hideConfigWindow()
{
    if (m_shellSurface) {
        //!NOTE: Avoid crash in wayland enviroment with qt5.9
        close();
    } else {
        hide();
    }
}

//!BEGIN borders
Plasma::FrameSvg::EnabledBorders DockSecConfigView::enabledBorders() const
{
    return m_enabledBorders;
}

void DockSecConfigView::updateEnabledBorders()
{
    if (!this->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;

    switch (m_dockView->location()) {
        case Plasma::Types::TopEdge:
            borders &= ~Plasma::FrameSvg::TopBorder;
            break;

        case Plasma::Types::LeftEdge:
            borders &= ~Plasma::FrameSvg::LeftBorder;
            break;

        case Plasma::Types::RightEdge:
            borders &= ~Plasma::FrameSvg::RightBorder;
            break;

        case Plasma::Types::BottomEdge:
            borders &=  ~Plasma::FrameSvg::BottomBorder;
            break;

        default:
            break;
    }

    if (m_enabledBorders != borders) {
        m_enabledBorders = borders;

        PanelShadows::self()->addWindow(this, m_enabledBorders);

        emit enabledBordersChanged();
    }
}

//!END borders


}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
