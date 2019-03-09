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

#include "secondaryconfigview.h"

// local
#include "primaryconfigview.h"
#include "../panelshadows_p.h"
#include "../view.h"
#include "../../lattecorona.h"
#include "../../wm/abstractwindowinterface.h"

// Qt
#include <QFontMetrics>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

// KDE
#include <KLocalizedContext>
#include <KDeclarative/KDeclarative>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem>

// Plasma
#include <Plasma/Package>

namespace Latte {
namespace ViewPart {

SecondaryConfigView::SecondaryConfigView(Latte::View *view, QWindow *parent)
    : QQuickView(nullptr),
      m_latteView(view)
{
    m_parent = qobject_cast<PrimaryConfigView *>(parent);
    m_corona = qobject_cast<Latte::Corona *>(m_latteView->containment()->corona());

    setupWaylandIntegration();

    setResizeMode(QQuickView::SizeViewToRootObject);
    setScreen(m_latteView->screen());

    if (m_latteView && m_latteView->containment()) {
        setIcon(qGuiApp->windowIcon());
    }

    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(100);

    connections << connect(&m_screenSyncTimer, &QTimer::timeout, this, [this]() {
        setScreen(m_latteView->screen());
        setFlags(wFlags());
        syncGeometry();
        syncSlideEffect();
    });
    connections << connect(m_latteView->visibility(), &VisibilityManager::modeChanged, this, &SecondaryConfigView::syncGeometry);

    m_thicknessSyncTimer.setSingleShot(true);
    m_thicknessSyncTimer.setInterval(200);
    connections << connect(&m_thicknessSyncTimer, &QTimer::timeout, this, [this]() {
        syncGeometry();
    });

    connections << connect(m_latteView, &Latte::View::normalThicknessChanged, [&]() {
        m_thicknessSyncTimer.start();
    });
}

SecondaryConfigView::~SecondaryConfigView()
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

void SecondaryConfigView::init()
{
    qDebug() << "dock secondary config view : initialization started...";

    setDefaultAlphaBuffer(true);
    setColor(Qt::transparent);
    PanelShadows::self()->addWindow(this);
    rootContext()->setContextProperty(QStringLiteral("latteView"), m_latteView);
    rootContext()->setContextProperty(QStringLiteral("viewConfig"), this);
    rootContext()->setContextProperty(QStringLiteral("plasmoid"), m_latteView->containment()->property("_plasma_graphicObject").value<QObject *>());

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.setTranslationDomain(QStringLiteral("latte-dock"));
    kdeclarative.setupBindings();

    QByteArray tempFilePath = "lattedocksecondaryconfigurationui";

    updateEnabledBorders();

    auto source = QUrl::fromLocalFile(m_latteView->containment()->corona()->kPackage().filePath(tempFilePath));
    setSource(source);
    syncGeometry();
    syncSlideEffect();

    if (m_parent) {
        m_parent->requestActivate();
    }

    qDebug() << "dock secondary config view : initialization ended...";
}

inline Qt::WindowFlags SecondaryConfigView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) & ~Qt::WindowDoesNotAcceptFocus;
}

QRect SecondaryConfigView::geometryWhenVisible() const
{
    return m_geometryWhenVisible;
}

void SecondaryConfigView::syncGeometry()
{
    if (!m_latteView || !m_latteView->managedLayout() || !m_latteView->containment() || !rootObject()) {
        return;
    }

    const QSize size(rootObject()->width(), rootObject()->height());
    setMaximumSize(size);
    setMinimumSize(size);
    resize(size);

    const auto location = m_latteView->containment()->location();
    const auto scrGeometry = m_latteView->screenGeometry();
    const auto availGeometry = m_parent->availableScreenGeometry();

    int clearThickness = m_latteView->editThickness();

    int secondaryConfigSpacing = 2 * m_latteView->fontPixelSize();

    QPoint position{0, 0};

    int xPos{0};
    int yPos{0};

    switch (m_latteView->containment()->formFactor()) {
    case Plasma::Types::Horizontal: {
        xPos = availGeometry.x() + secondaryConfigSpacing;

        if (location == Plasma::Types::TopEdge) {
            yPos = scrGeometry.y() + clearThickness;
        } else if (location == Plasma::Types::BottomEdge) {
            yPos = scrGeometry.y() + scrGeometry.height() - clearThickness - size.height();
        }
    }
        break;

    case Plasma::Types::Vertical: {
        yPos = availGeometry.y() + secondaryConfigSpacing;

        if (location == Plasma::Types::LeftEdge) {
            xPos = scrGeometry.x() + clearThickness;
        } else if (location == Plasma::Types::RightEdge) {
            xPos = scrGeometry.x() + scrGeometry.width() - clearThickness - size.width();
        }
    }
        break;

    default:
        qWarning() << "no sync geometry, wrong formFactor";
        break;
    }

    position = {xPos, yPos};

    updateEnabledBorders();

    m_geometryWhenVisible = QRect(position.x(), position.y(), size.width(), size.height());

    setPosition(position);

    if (m_shellSurface) {
        m_shellSurface->setPosition(position);
    }

    //! after placement request to activate the main config window in order to avoid
    //! rare cases of closing settings window from secondaryConfigView->focusOutEvent
    if (m_parent) {
        m_parent->requestActivate();
    }
}

void SecondaryConfigView::syncSlideEffect()
{
    if (!m_latteView || !m_latteView->containment()) {
        return;
    }

    auto slideLocation = WindowSystem::Slide::None;

    switch (m_latteView->containment()->location()) {
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

void SecondaryConfigView::showEvent(QShowEvent *ev)
{
    QQuickWindow::showEvent(ev);

    if (!m_latteView) {
        return;
    }

    m_corona->wm()->setViewExtraFlags(*this);
    setFlags(wFlags());

    m_corona->wm()->enableBlurBehind(*this);

    syncGeometry();
    syncSlideEffect();

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &SecondaryConfigView::syncGeometry);

    emit showSignal();
}

void SecondaryConfigView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);

    const auto *focusWindow = qGuiApp->focusWindow();

    if ((focusWindow && (focusWindow->flags().testFlag(Qt::Popup)
                         || focusWindow->flags().testFlag(Qt::ToolTip)))
            || m_latteView->alternativesIsShown()) {
        return;
    }

    const auto parent = qobject_cast<PrimaryConfigView *>(m_parent);

    if (!m_latteView->containsMouse() && parent && !parent->sticker() && !parent->isActive()) {
        parent->hideConfigWindow();
    }
}

void SecondaryConfigView::setupWaylandIntegration()
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

        qDebug() << "wayland dock window surface was created...";

        m_shellSurface = interface->createSurface(s, this);
        m_shellSurface->setSkipTaskbar(true);

        syncGeometry();
    }
}

bool SecondaryConfigView::event(QEvent *e)
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

void SecondaryConfigView::hideConfigWindow()
{
    if (m_shellSurface) {
        //!NOTE: Avoid crash in wayland environment with qt5.9
        close();
    } else {
        hide();
    }
}

//!BEGIN borders
Plasma::FrameSvg::EnabledBorders SecondaryConfigView::enabledBorders() const
{
    return m_enabledBorders;
}

void SecondaryConfigView::updateEnabledBorders()
{
    if (!this->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;

    switch (m_latteView->location()) {
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
}

