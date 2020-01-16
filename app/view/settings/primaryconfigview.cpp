/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
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

#include "primaryconfigview.h"

// local
#include <config-latte.h>
#include "secondaryconfigview.h"
#include "../effects.h"
#include "../panelshadows_p.h"
#include "../view.h"
#include "../../lattecorona.h"
#include "../../layouts/manager.h"
#include "../../layout/genericlayout.h"
#include "../../settings/universalsettings.h"
#include "../../shortcuts/globalshortcuts.h"
#include "../../shortcuts/shortcutstracker.h"
#include "../../wm/abstractwindowinterface.h"

// Qt
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

// KDE
#include <KLocalizedContext>
#include <KDeclarative/KDeclarative>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowEffects>
#include <KWindowSystem>

// Plasma
#include <Plasma/Package>

namespace Latte {
namespace ViewPart {

PrimaryConfigView::PrimaryConfigView(Plasma::Containment *containment, Latte::View *view, QWindow *parent)
    : PlasmaQuick::ConfigView(containment, parent),
      m_latteView(view)
{
    m_corona = qobject_cast<Latte::Corona *>(m_latteView->containment()->corona());

    setupWaylandIntegration();

    if (KWindowSystem::isPlatformX11()) {
        m_corona->wm()->registerIgnoredWindow(winId());
    } else {
        connect(m_corona->wm(), &WindowSystem::AbstractWindowInterface::latteWindowAdded, this, [&]() {
            if (m_waylandWindowId.isNull()) {
                m_waylandWindowId = m_corona->wm()->winIdFor("latte-dock", geometry());
                m_corona->wm()->registerIgnoredWindow(m_waylandWindowId);
            }
        });
    }

    setScreen(m_latteView->screen());

    if (containment) {
        setIcon(qGuiApp->windowIcon());
    }

    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(100);

    connect(this, &PrimaryConfigView::availableScreenGeometryChanged, this, &PrimaryConfigView::syncGeometry);
    connect(this, &PrimaryConfigView::complexityChanged, this, &PrimaryConfigView::saveConfig);
    connect(this, &PrimaryConfigView::complexityChanged, this, &PrimaryConfigView::updateShowInlineProperties);
    connect(this, &PrimaryConfigView::complexityChanged, this, &PrimaryConfigView::syncGeometry);

    connect(this, &QQuickView::statusChanged, [&](QQuickView::Status status) {
        if (status == QQuickView::Ready) {
            updateEffects();
        }
    });

    connections << connect(&m_screenSyncTimer, &QTimer::timeout, this, [this]() {
        setScreen(m_latteView->screen());
        setFlags(wFlags());

        if (KWindowSystem::isPlatformX11()) {
            m_corona->wm()->setViewExtraFlags(this, false, Latte::Types::NormalWindow);
        }

        syncGeometry();
        syncSlideEffect();
    });
    connections << connect(m_latteView->visibility(), &VisibilityManager::modeChanged, this, &PrimaryConfigView::syncGeometry);
    connections << connect(containment, &Plasma::Containment::immutabilityChanged, this, &PrimaryConfigView::immutabilityChanged);

    m_thicknessSyncTimer.setSingleShot(true);
    m_thicknessSyncTimer.setInterval(200);
    connections << connect(&m_thicknessSyncTimer, &QTimer::timeout, this, [this]() {
        syncGeometry();
    });

    connections << connect(m_latteView, &Latte::View::normalThicknessChanged, [&]() {
        m_thicknessSyncTimer.start();
    });

    if (m_corona) {
        connections << connect(m_corona, &Latte::Corona::raiseViewsTemporaryChanged, this, &PrimaryConfigView::raiseDocksTemporaryChanged);
        connections << connect(m_corona, &Latte::Corona::availableScreenRectChangedFrom, this, &PrimaryConfigView::updateAvailableScreenGeometry);
    }

    if (m_latteView->layout()) {
        emit m_latteView->layout()->setLastConfigViewFor(m_latteView);
    }
}

PrimaryConfigView::~PrimaryConfigView()
{
    qDebug() << "ConfigView deleting ...";

    m_corona->dialogShadows()->removeWindow(this);

    m_corona->wm()->unregisterIgnoredWindow(KWindowSystem::isPlatformX11() ? winId() : m_waylandWindowId);

    deleteSecondaryWindow();

    for (const auto &var : connections) {
        QObject::disconnect(var);
    }
}

void PrimaryConfigView::init()
{
    qDebug() << "dock config view : initialization started...";
    m_originalByPassWM = m_latteView->byPassWM();
    m_originalMode = m_latteView->visibility()->mode();

    loadConfig();

    setDefaultAlphaBuffer(true);
    setColor(Qt::transparent);
    m_corona->dialogShadows()->addWindow(this);
    rootContext()->setContextProperty(QStringLiteral("latteView"), m_latteView);
    rootContext()->setContextProperty(QStringLiteral("shortcutsEngine"), m_corona->globalShortcuts()->shortcutsTracker());
    rootContext()->setContextProperty(QStringLiteral("viewConfig"), this);

    if (m_corona) {
        rootContext()->setContextProperty(QStringLiteral("universalSettings"), m_corona->universalSettings());
        rootContext()->setContextProperty(QStringLiteral("layoutsManager"), m_corona->layoutsManager());
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

    QByteArray tempFilePath = "lattedockconfigurationui";

    updateEnabledBorders();
    updateAvailableScreenGeometry();

    auto source = QUrl::fromLocalFile(m_latteView->containment()->corona()->kPackage().filePath(tempFilePath));
    setSource(source);
    syncGeometry();
    syncSlideEffect();

    qDebug() << "dock config view : initialization ended...";
}

inline Qt::WindowFlags PrimaryConfigView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint) & ~Qt::WindowDoesNotAcceptFocus;
}

QQuickView *PrimaryConfigView::secondaryWindow()
{
    return m_secConfigView;
}

void PrimaryConfigView::createSecondaryWindow()
{
    if (m_secConfigView) {
        return;
    }

    m_secConfigView = new SecondaryConfigView(m_latteView, this);
    m_secConfigView->init();
}

void PrimaryConfigView::deleteSecondaryWindow()
{
    if (m_secConfigView) {
        auto secWindow = m_secConfigView;
        m_secConfigView = nullptr;
        secWindow->deleteLater();

        if (KWindowSystem::isPlatformX11()) {
            //! this is needed in order for subtracked mask of secondary window to
            //! be released properly when changing for Advanced to Basic mode.
            //! Under wayland this is not needed because masks do not break any visuals.
            m_latteView->effects()->updateMask();
        }
    }
}

void PrimaryConfigView::updateAvailableScreenGeometry(View *origin)
{    
    int currentScrId = m_latteView->positioner()->currentScreenId();
    m_availableScreenGeometry = m_corona->availableScreenRectWithCriteria(currentScrId, QString(), {}, {}, true);

    emit availableScreenGeometryChanged();
}

QRect PrimaryConfigView::availableScreenGeometry() const
{
    return m_availableScreenGeometry;
}

QRect PrimaryConfigView::geometryWhenVisible() const
{
    return m_geometryWhenVisible;
}

void PrimaryConfigView::requestActivate()
{
    if (KWindowSystem::isPlatformWayland() && m_shellSurface) {
        if (m_waylandWindowId.isNull()) {
            m_waylandWindowId = m_corona->wm()->winIdFor("latte-dock", geometry());
        }

        m_corona->wm()->requestActivate(m_waylandWindowId);
    } else {
        QQuickView::requestActivate();
    }
}

void PrimaryConfigView::syncGeometry()
{
    if (!m_latteView || !m_latteView->layout() || !m_latteView->containment() || !rootObject()) {
        return;
    }

    const QSize size(rootObject()->width(), rootObject()->height());
    setMaximumSize(size);
    setMinimumSize(size);
    resize(size);

    const auto location = m_latteView->containment()->location();
    const auto scrGeometry = m_latteView->screenGeometry();
    const auto availGeometry = m_availableScreenGeometry;

    int clearThickness = m_latteView->editThickness();

    QPoint position{0, 0};

    int xPos{0};
    int yPos{0};

    switch (m_latteView->formFactor()) {
    case Plasma::Types::Horizontal: {
        if (m_complexity == Latte::Types::ExpertSettings) {
            if (qApp->isLeftToRight()) {
                xPos = availGeometry.x() + availGeometry.width() - size.width();
            } else {
                xPos = availGeometry.x();
            }
        } else {
            xPos = scrGeometry.center().x() - size.width() / 2;
        }

        if (location == Plasma::Types::TopEdge) {
            yPos = scrGeometry.y() + clearThickness;
        } else if (location == Plasma::Types::BottomEdge) {
            yPos = scrGeometry.y() + scrGeometry.height() - clearThickness - size.height();
        }
    }
        break;

    case Plasma::Types::Vertical: {
        if (location == Plasma::Types::LeftEdge) {
            xPos = scrGeometry.x() + clearThickness;
            yPos =  availGeometry.y() + (availGeometry.height() - size.height())/2;
        } else if (location == Plasma::Types::RightEdge) {
            xPos = scrGeometry.x() + scrGeometry.width() - clearThickness - size.width();
            yPos =  availGeometry.y() + (availGeometry.height() - size.height())/2;
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

    updateShowInlineProperties();

    emit m_latteView->configWindowGeometryChanged();
}

void PrimaryConfigView::syncSlideEffect()
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

void PrimaryConfigView::showEvent(QShowEvent *ev)
{
    QQuickWindow::showEvent(ev);

    if (!m_latteView) {
        return;
    }

    setFlags(wFlags());
    m_corona->wm()->setViewExtraFlags(this, false, Latte::Types::NormalWindow);

    syncGeometry();
    syncSlideEffect();

    if (m_latteView && m_latteView->containment())
        m_latteView->containment()->setUserConfiguring(true);

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &PrimaryConfigView::syncGeometry);

    emit showSignal();
}

void PrimaryConfigView::hideEvent(QHideEvent *ev)
{
    if (!m_latteView) {
        deleteLater();
        //QQuickWindow::hideEvent(ev);
        return;
    }

    if (m_latteView->containment()) {
        m_latteView->containment()->setUserConfiguring(false);
    }

    // QQuickWindow::hideEvent(ev);

    const auto mode = m_latteView->visibility()->mode();

    if ((mode == Types::AlwaysVisible || mode == Types::WindowsGoBelow)
            && !(m_originalMode == Types::AlwaysVisible || m_originalMode == Types::WindowsGoBelow)) {
        //! mode changed to AlwaysVisible OR WindowsGoBelow FROM Dodge mode
        if (m_originalByPassWM) {
            //! if original by pass is active
            m_latteView->layout()->recreateView(m_latteView->containment());
        }
    } else if (m_latteView->byPassWM() != m_originalByPassWM) {
        m_latteView->layout()->recreateView(m_latteView->containment());
    }

    deleteLater();
}

void PrimaryConfigView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);

    const auto *focusWindow = qGuiApp->focusWindow();

    if (!m_latteView
            || (focusWindow && (focusWindow->flags().testFlag(Qt::Popup)
                                || focusWindow->flags().testFlag(Qt::ToolTip)))
            || m_latteView->alternativesIsShown()) {
        return;
    }

    if (!m_blockFocusLost && !m_latteView->containsMouse()
            && (!m_secConfigView || (m_secConfigView && !m_secConfigView->isActive()))) {
        hideConfigWindow();
    }
}

void PrimaryConfigView::setupWaylandIntegration()
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
        m_corona->wm()->setViewExtraFlags(m_shellSurface, false);

        syncGeometry();
    }
}

bool PrimaryConfigView::event(QEvent *e)
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

    return PlasmaQuick::ConfigView::event(e);
}


void PrimaryConfigView::immutabilityChanged(Plasma::Types::ImmutabilityType type)
{
    if (type != Plasma::Types::Mutable && isVisible())
        hideConfigWindow();
}

bool PrimaryConfigView::sticker() const
{
    return m_blockFocusLost;
}

void PrimaryConfigView::setSticker(bool blockFocusLost)
{
    if (m_blockFocusLost == blockFocusLost)
        return;

    m_blockFocusLost = blockFocusLost;
}

bool PrimaryConfigView::showInlineProperties() const
{
    return m_showInlineProperties;
}
void PrimaryConfigView::setShowInlineProperties(bool show)
{
    if (m_showInlineProperties == show) {
        return;
    }

    m_showInlineProperties = show;
    emit showInlinePropertiesChanged();
}

void PrimaryConfigView::updateShowInlineProperties()
{
    if (!m_latteView) {
        return;
    }

    bool showSecWindow{false};
    bool complexityApprovedSecWindow{false};


    if (m_complexity != Latte::Types::BasicSettings
            && !(m_complexity == Latte::Types::ExpertSettings && m_latteView->formFactor() == Plasma::Types::Vertical)) {
        showSecWindow = true;
        complexityApprovedSecWindow = true;
    }

    //! consider screen geometry for showing or not the secondary window
    if (!geometryWhenVisible().isNull()) {
        createSecondaryWindow();

        if (m_secConfigView->geometryWhenVisible().intersects(geometryWhenVisible())) {
            showSecWindow = false;
        } else if (complexityApprovedSecWindow) {
            showSecWindow = true;
        }
    }

    if (showSecWindow) {
        if (!m_secConfigView) {
            createSecondaryWindow();
        }

        QTimer::singleShot(150, m_secConfigView, SLOT(show()));
        setShowInlineProperties(false);
    } else {
        deleteSecondaryWindow();
        setShowInlineProperties(true);
    }

    // qDebug() << " showSecWindow:" << showSecWindow << " _ " << " inline:"<< !showSecWindow;
}

int PrimaryConfigView::complexity() const
{
    return (int)m_complexity;
}

void PrimaryConfigView::setComplexity(int complexity)
{
    if ((int)m_complexity == complexity) {
        return;
    }

    m_complexity = static_cast<Latte::Types::SettingsComplexity>(complexity);

    emit complexityChanged();
}

void PrimaryConfigView::hideConfigWindow()
{
    if (m_shellSurface) {
        //!NOTE: Avoid crash in wayland environment with qt5.9
        close();
    } else {
        hide();
    }
}

void PrimaryConfigView::updateLaunchersForGroup(int groupInt)
{
    Types::LaunchersGroup group = (Types::LaunchersGroup)groupInt;

    //! when the layout/global launchers list is empty then the current dock launchers are used for them
    //! as a start point
    if (m_corona &&  m_latteView->layout()) {
        if ((group == Types::LayoutLaunchers && m_latteView->layout()->launchers().isEmpty())
                || (group == Types::GlobalLaunchers && m_corona->universalSettings()->launchers().isEmpty())) {

            Plasma::Containment *c = m_latteView->containment();

            const auto &applets = c->applets();

            for (auto *applet : applets) {
                KPluginMetaData meta = applet->kPackage().metadata();

                if (meta.pluginId() == "org.kde.latte.plasmoid") {
                    if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
                        const auto &childItems = appletInterface->childItems();

                        if (childItems.isEmpty()) {
                            continue;
                        }

                        for (QQuickItem *item : childItems) {
                            if (auto *metaObject = item->metaObject()) {
                                // not using QMetaObject::invokeMethod to avoid warnings when calling
                                // this on applets that don't have it or other child items since this
                                // is pretty much trial and error.
                                // Also, "var" arguments are treated as QVariant in QMetaObject

                                int methodIndex = metaObject->indexOfMethod("getLauncherList()");

                                if (methodIndex == -1) {
                                    continue;
                                }

                                QMetaMethod method = metaObject->method(methodIndex);

                                QVariant launchers;

                                if (method.invoke(item, Q_RETURN_ARG(QVariant, launchers))) {
                                    if (group == Types::LayoutLaunchers) {
                                        m_latteView->layout()->setLaunchers(launchers.toStringList());
                                    } else if (group == Types::GlobalLaunchers) {
                                        m_corona->universalSettings()->setLaunchers(launchers.toStringList());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

//!BEGIN borders
Plasma::FrameSvg::EnabledBorders PrimaryConfigView::enabledBorders() const
{
    return m_enabledBorders;
}

void PrimaryConfigView::updateEnabledBorders()
{
    if (!this->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;

    switch (m_latteView->location()) {
    case Plasma::Types::TopEdge:
        borders &= m_inReverse ? ~Plasma::FrameSvg::BottomBorder : ~Plasma::FrameSvg::TopBorder;
        break;

    case Plasma::Types::LeftEdge:
        borders &= ~Plasma::FrameSvg::LeftBorder;
        break;

    case Plasma::Types::RightEdge:
        borders &= ~Plasma::FrameSvg::RightBorder;
        break;

    case Plasma::Types::BottomEdge:
        borders &= m_inReverse ? ~Plasma::FrameSvg::TopBorder : ~Plasma::FrameSvg::BottomBorder;
        break;

    default:
        break;
    }

    if (m_enabledBorders != borders) {
        m_enabledBorders = borders;

        m_corona->dialogShadows()->addWindow(this, m_enabledBorders);

        emit enabledBordersChanged();
    }
}
//!END borders

void PrimaryConfigView::updateEffects()
{
    //! Don't apply any effect before the wayland surface is created under wayland
    //! https://bugs.kde.org/show_bug.cgi?id=392890
    if (KWindowSystem::isPlatformWayland() && !m_shellSurface) {
        return;
    }

    QRegion mask;

    QQuickItem *rootObject = this->rootObject();
    if (rootObject) {
        const QVariant maskProperty = rootObject->property("backgroundMask");
        if (static_cast<QMetaType::Type>(maskProperty.type()) == QMetaType::QRegion) {
            mask = maskProperty.value<QRegion>();
        }
    }

    if (!mask.isEmpty()) {
        setMask(mask);
        KWindowEffects::enableBlurBehind(winId(), true, mask);
    } else {
        setMask(QRect());
        KWindowEffects::enableBlurBehind(winId(), false);
    }
}

//!BEGIN configuration
void PrimaryConfigView::loadConfig()
{
    if (!m_latteView || !m_latteView->containment()) {
        return;
    }
    auto config = m_latteView->containment()->config();
    int complexity = config.readEntry("settingsComplexity", (int)Latte::Types::BasicSettings);
    setComplexity(static_cast<Latte::Types::SettingsComplexity>(complexity));
}

void PrimaryConfigView::saveConfig()
{
    if (!m_latteView || !m_latteView->containment()) {
        return;
    }

    auto config = m_latteView->containment()->config();
    config.writeEntry("settingsComplexity", (int)m_complexity);
    config.sync();
}
//!END configuration

}
}

