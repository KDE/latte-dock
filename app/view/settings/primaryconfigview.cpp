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

    setTitle(validTitle());

    if (KWindowSystem::isPlatformX11()) {
        m_corona->wm()->registerIgnoredWindow(winId());
    } else {
        connect(m_corona->wm(), &WindowSystem::AbstractWindowInterface::latteWindowAdded, this, &PrimaryConfigView::updateWaylandId);
    }

    setScreen(m_latteView->screen());

    if (containment) {
        setIcon(qGuiApp->windowIcon());
    }

    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(100);

    connect(this, &QQuickView::widthChanged, this, &PrimaryConfigView::updateEffects);
    connect(this, &QQuickView::heightChanged, this, &PrimaryConfigView::updateEffects);

    connect(this, &PrimaryConfigView::availableScreenGeometryChanged, this, &PrimaryConfigView::syncGeometry);
    connect(this, &PrimaryConfigView::inAdvancedModeChanged, this, &PrimaryConfigView::saveConfig);
    connect(this, &PrimaryConfigView::inAdvancedModeChanged, this, &PrimaryConfigView::updateShowInlineProperties);
    connect(this, &PrimaryConfigView::inAdvancedModeChanged, this, &PrimaryConfigView::syncGeometry);

    connect(this, &PrimaryConfigView::inAdvancedModeChanged, m_latteView, &Latte::View::inSettingsAdvancedModeChanged);

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

    connections << connect(m_latteView, &View::hiddenConfigurationWindowsAreDeletedChanged, this, &PrimaryConfigView::onHiddenConfigurationWindowsAreDeletedChanged);
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
}

PrimaryConfigView::~PrimaryConfigView()
{
    qDebug() << "ConfigView deleting ...";

    if (m_latteView->indicator()) {
        //! destroy indicator config ui when the configuration window is closed
        m_latteView->indicator()->releaseConfigUi();
    }

    m_corona->dialogShadows()->removeWindow(this);

    m_corona->wm()->unregisterIgnoredWindow(KWindowSystem::isPlatformX11() ? winId() : m_waylandWindowId);

    if (m_secConfigView) {
        m_secConfigView->deleteLater();
    }

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
    //! inform view about the current settings level
    emit m_latteView->inSettingsAdvancedModeChanged();

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

QString PrimaryConfigView::validTitle() const
{
    return QString("#primaryconfig#" + QString::number(m_latteView->containment()->id()));
}

QQuickView *PrimaryConfigView::secondaryWindow()
{
    return m_secConfigView;
}

void PrimaryConfigView::showSecondaryWindow()
{       
    if (!m_secConfigView) {
        m_secConfigView = new SecondaryConfigView(m_latteView, this);
        m_secConfigView->init();
    } else if (m_secConfigView && !m_latteView->hiddenConfigurationWindowsAreDeleted() && !m_secConfigView->isVisible()){
        m_secConfigView->show();
    }
}

void PrimaryConfigView::hideSecondaryWindow()
{
    if (m_secConfigView) {
        if (m_latteView->hiddenConfigurationWindowsAreDeleted()) {
            auto secWindow = m_secConfigView;
            m_secConfigView = nullptr;
            secWindow->deleteLater();
        } else {
            m_secConfigView->hide();
        }

        if (KWindowSystem::isPlatformX11() && m_latteView->effects()) {
            //! this is needed in order for subtracked mask of secondary window to
            //! be released properly when changing for Advanced to Basic mode.
            //! Under wayland this is not needed because masks do not break any visuals.
            m_latteView->effects()->updateMask();
        }
    }
}

void PrimaryConfigView::onHiddenConfigurationWindowsAreDeletedChanged()
{
    if (m_latteView && m_latteView->hiddenConfigurationWindowsAreDeleted() && !isVisible()) {
        deleteLater();
    }
}

void PrimaryConfigView::updateAvailableScreenGeometry(View *origin)
{    
    int currentScrId = m_latteView->positioner()->currentScreenId();

    QList<Latte::Types::Visibility> ignoreModes{Latte::Types::SideBar};

    if (m_latteView->visibility() && m_latteView->visibility()->mode() == Latte::Types::SideBar) {
        ignoreModes.removeAll(Latte::Types::SideBar);
    }

    m_availableScreenGeometry = m_corona->availableScreenRectWithCriteria(currentScrId, QString(), ignoreModes, {}, false);

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
        updateWaylandId();
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
    const auto location = m_latteView->containment()->location();
    const auto scrGeometry = m_latteView->screenGeometry();
    const auto availGeometry = m_availableScreenGeometry;

    int clearThickness = m_latteView->editThickness();

    QPoint position{0, 0};

    int xPos{0};
    int yPos{0};

    switch (m_latteView->formFactor()) {
    case Plasma::Types::Horizontal: {
        if (m_inAdvancedMode) {
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

    auto geometry = QRect(position.x(), position.y(), size.width(), size.height());

    if (m_geometryWhenVisible == geometry) {
        return;
    }

    m_geometryWhenVisible = geometry;

    setPosition(position);

    if (m_shellSurface) {
        m_shellSurface->setPosition(position);
    }

    setMaximumSize(size);
    setMinimumSize(size);
    resize(size);

    updateViewMask();
    emit m_latteView->configWindowGeometryChanged();
}

void PrimaryConfigView::updateViewMask()
{
    bool environmentState = (KWindowSystem::isPlatformX11() && KWindowSystem::compositingActive());

    if (!environmentState) {
        return;
    }

    int x, y, thickness, length;
    QRegion area;

    thickness = m_latteView->effects()->editShadow();

    if (m_latteView->formFactor() == Plasma::Types::Vertical) {
        length = m_geometryWhenVisible.height();
    } else {
        length = m_geometryWhenVisible.width();
    }

    if (m_latteView->formFactor() == Plasma::Types::Horizontal) {
        x = m_geometryWhenVisible.x() - m_latteView->x();
    } else {
        y = m_geometryWhenVisible.y() - m_latteView->y();
    }

    if (m_latteView->location() == Plasma::Types::BottomEdge) {
        y = m_latteView->height() - m_latteView->editThickness() - m_latteView->effects()->editShadow();
    } else if (m_latteView->location() == Plasma::Types::TopEdge) {
        y = m_latteView->editThickness();
    } else if (m_latteView->location() == Plasma::Types::LeftEdge) {
        x = m_latteView->editThickness();
    } else if (m_latteView->location() == Plasma::Types::RightEdge) {
        x = m_latteView->width() - m_latteView->editThickness() - m_latteView->effects()->editShadow();
    }

    if (m_latteView->formFactor() == Plasma::Types::Horizontal) {
        area = QRect(x, y, length, thickness);
    } else {
        area = QRect(x, y, thickness, length);
    }

    m_latteView->effects()->setSubtractedMaskRegion(validTitle(), area);
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

    if (m_latteView && m_latteView->containment()) {
        m_latteView->containment()->setUserConfiguring(true);
    }

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &PrimaryConfigView::syncGeometry);

    updateShowInlineProperties();
    updateViewMask();

    emit showSignal();

    if (m_latteView && m_latteView->layout()) {
        m_latteView->layout()->setLastConfigViewFor(m_latteView);
    }

    if (m_shellSurface) {
        //! readd shadows after hiding because the window shadows are not shown again after first showing
        m_corona->dialogShadows()->addWindow(this, m_enabledBorders);
    }
}

void PrimaryConfigView::hideEvent(QHideEvent *ev)
{
    if (!m_latteView) {
        deleteLater();
        return;
    }

    if (m_latteView->containment()) {
        m_latteView->containment()->setUserConfiguring(false);
    }

    m_latteView->effects()->removeSubtractedMaskRegion(validTitle());

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

    if (m_latteView->hiddenConfigurationWindowsAreDeleted()) {
        deleteLater();
    } else {
        setVisible(false);
    }
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
    bool advancedApprovedSecWindow{false};

    if (m_inAdvancedMode && m_latteView->formFactor() != Plasma::Types::Vertical) {
        showSecWindow = true;
        advancedApprovedSecWindow = true;
    }

    //! consider screen geometry for showing or not the secondary window
    if (showSecWindow && !geometryWhenVisible().isNull()) {
        if (m_secConfigView && m_secConfigView->geometryWhenVisible().intersects(geometryWhenVisible())) {
            showSecWindow = false;
        } else if (advancedApprovedSecWindow) {
            showSecWindow = true;
        }
    }

    if (showSecWindow) {
        showSecondaryWindow();

       // QTimer::singleShot(150, m_secConfigView, SLOT(show()));
        setShowInlineProperties(false);
    } else {
        hideSecondaryWindow();
        setShowInlineProperties(true);
    }

    // qDebug() << " showSecWindow:" << showSecWindow << " _ " << " inline:"<< !showSecWindow;
}

void PrimaryConfigView::updateWaylandId()
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

bool PrimaryConfigView::inAdvancedMode() const
{
    return m_inAdvancedMode;
}

void PrimaryConfigView::setInAdvancedMode(bool advanced)
{
    if (m_inAdvancedMode == advanced) {
        return;
    }

    m_inAdvancedMode = advanced;
    emit inAdvancedModeChanged();
}

void PrimaryConfigView::hideConfigWindow()
{
    if (m_shellSurface) {
        //!NOTE: Avoid crash in wayland environment with qt5.9
        close();
    } else {
        hide();
    }

    hideSecondaryWindow();
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

    if (!m_background) {
        m_background = new Plasma::FrameSvg(this);
    }

    if (m_background->imagePath() != "dialogs/background") {
        m_background->setImagePath(QStringLiteral("dialogs/background"));
    }

    m_background->setEnabledBorders(m_enabledBorders);
    m_background->resizeFrame(size());

    QRegion mask = m_background->mask();

    QRegion fixedMask = mask.isNull() ? QRegion(QRect(0,0,width(),height())) : mask;

    if (!fixedMask.isEmpty()) {
        setMask(fixedMask);
    } else {
        setMask(QRegion());
    }

    if (KWindowSystem::compositingActive()) {
        KWindowEffects::enableBlurBehind(winId(), true, fixedMask);
    } else {
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
    int complexity = config.readEntry("settingsComplexity", 0);
    setInAdvancedMode(complexity>0);
}

void PrimaryConfigView::saveConfig()
{
    if (!m_latteView || !m_latteView->containment()) {
        return;
    }

    auto config = m_latteView->containment()->config();
    int complexity = m_inAdvancedMode ? 1 : 0;
    config.writeEntry("settingsComplexity", complexity);
}
//!END configuration

}
}

