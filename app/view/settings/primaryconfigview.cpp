/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "primaryconfigview.h"

// local
#include <config-latte.h>
#include "canvasconfigview.h"
#include "indicatoruimanager.h"
#include "secondaryconfigview.h"
#include "../effects.h"
#include "../panelshadows_p.h"
#include "../view.h"
#include "../../lattecorona.h"
#include "../../layouts/manager.h"
#include "../../layout/genericlayout.h"
#include "../../settings/universalsettings.h"
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

#define CANVASWINDOWINTERVAL 50
#define PRIMARYWINDOWINTERVAL 250
#define SECONDARYWINDOWINTERVAL 200
#define SLIDEOUTINTERVAL 400

namespace Latte {
namespace ViewPart {

PrimaryConfigView::PrimaryConfigView(Latte::View *view)
    : SubConfigView(view, QString("#primaryconfigview#")),
      m_indicatorUiManager(new Config::IndicatorUiManager(this))
{
    connect(this, &QQuickWindow::xChanged, this, &PrimaryConfigView::xChanged);
    connect(this, &QQuickWindow::yChanged, this, &PrimaryConfigView::yChanged);

    connect(this, &QQuickView::widthChanged, this, &PrimaryConfigView::updateEffects);
    connect(this, &QQuickView::heightChanged, this, &PrimaryConfigView::updateEffects);

    connect(this, &PrimaryConfigView::availableScreenGeometryChanged, this, &PrimaryConfigView::syncGeometry);

    connect(this, &QQuickView::statusChanged, [&](QQuickView::Status status) {
        if (status == QQuickView::Ready) {
            updateEffects();
        }
    });

    if (m_corona) {
        connections << connect(m_corona, &Latte::Corona::raiseViewsTemporaryChanged, this, &PrimaryConfigView::raiseDocksTemporaryChanged);
        connections << connect(m_corona, &Latte::Corona::availableScreenRectChangedFrom, this, &PrimaryConfigView::updateAvailableScreenGeometry);

        connections << connect(m_corona->layoutsManager(), &Latte::Layouts::Manager::currentLayoutIsSwitching, this, [this]() {
            if (isVisible()) {
                hideConfigWindow();
            }
        });

        connect(m_corona->universalSettings(), &Latte::UniversalSettings::inAdvancedModeForEditSettingsChanged,
                this, &PrimaryConfigView::updateShowInlineProperties);
        connect(m_corona->universalSettings(), &Latte::UniversalSettings::inAdvancedModeForEditSettingsChanged,
                this, &PrimaryConfigView::syncGeometry);
    }

    m_availableScreemGeometryTimer.setSingleShot(true);
    m_availableScreemGeometryTimer.setInterval(250);

    connections << connect(&m_availableScreemGeometryTimer, &QTimer::timeout, this, [this]() {
        instantUpdateAvailableScreenGeometry();
    });

    setParentView(view);
    init();
}

PrimaryConfigView::~PrimaryConfigView()
{
    if (m_canvasConfigView) {
        delete m_canvasConfigView;
    }

    if (m_secConfigView) {
        delete m_secConfigView;
    }
}

void PrimaryConfigView::init()
{
    SubConfigView::init();

    QByteArray tempFilePath = "lattedockconfigurationui";

    auto source = QUrl::fromLocalFile(m_latteView->containment()->corona()->kPackage().filePath(tempFilePath));
    setSource(source);
    syncGeometry();
}

Config::IndicatorUiManager *PrimaryConfigView::indicatorUiManager()
{
    return m_indicatorUiManager;
}

void PrimaryConfigView::setOnActivities(QStringList activities)
{
    m_corona->wm()->setWindowOnActivities(trackedWindowId(), activities);

    if (m_secConfigView) {
        m_corona->wm()->setWindowOnActivities(m_secConfigView->trackedWindowId(), activities);
    }

    if (m_canvasConfigView) {
        m_corona->wm()->setWindowOnActivities(m_canvasConfigView->trackedWindowId(), activities);
    }
}

void PrimaryConfigView::requestActivate()
{
    if (m_latteView && m_latteView->visibility()) {
        if (KWindowSystem::isPlatformX11()) {
            m_latteView->visibility()->setViewOnFrontLayer();
        } else if (m_shellSurface) {
            m_corona->wm()->requestActivate(m_latteView->positioner()->trackedWindowId());
        }
    }

    if (m_secConfigView) {
        m_secConfigView->requestActivate();
    }

    SubConfigView::requestActivate();
}

void PrimaryConfigView::showConfigWindow()
{
    if (isVisible()) {
        return;
    }

    if (m_latteView && m_latteView->containment()) {
        m_latteView->containment()->setUserConfiguring(true);
    }

    showAfter(PRIMARYWINDOWINTERVAL);
    showCanvasWindow();
    showSecondaryWindow();
}

void PrimaryConfigView::hideConfigWindow()
{
    if (m_shellSurface) {
        //!NOTE: Avoid crash in wayland environment with qt5.9
        close();
    } else {
        hide();
    }

    hideCanvasWindow();
    hideSecondaryWindow();
}

void PrimaryConfigView::showCanvasWindow()
{
    if (!m_canvasConfigView) {
        m_canvasConfigView = new CanvasConfigView(m_latteView, this);
    }

    if (m_canvasConfigView && !m_canvasConfigView->isVisible()){
        m_canvasConfigView->showAfter(CANVASWINDOWINTERVAL);
    }
}

void PrimaryConfigView::hideCanvasWindow()
{
    if (m_canvasConfigView) {
        m_canvasConfigView->hideConfigWindow();
    }
}

void PrimaryConfigView::showSecondaryWindow()
{
    bool isValidShowing{m_latteView->formFactor() == Plasma::Types::Horizontal && inAdvancedMode()};

    if (!isValidShowing) {
        return;
    }

    if (!m_secConfigView) {
        m_secConfigView = new SecondaryConfigView(m_latteView, this);
    }

    if (m_secConfigView && !m_secConfigView->isVisible()){
        m_secConfigView->showAfter(SECONDARYWINDOWINTERVAL);
    }
}

void PrimaryConfigView::hideSecondaryWindow()
{
    if (m_secConfigView) {
        m_secConfigView->hideConfigWindow();
    }
}

void PrimaryConfigView::setParentView(Latte::View *view, const bool &immediate)
{
    if (m_latteView == view) {
        return;
    }

    if (m_latteView && !immediate) {
        hideConfigWindow();

        //!slide-out delay
        QTimer::singleShot(SLIDEOUTINTERVAL, [this, view]() {
            initParentView(view);
            showConfigWindow();
        });
    } else {
        initParentView(view);
        showConfigWindow();
    }
}

void PrimaryConfigView::initParentView(Latte::View *view)
{
    setIsReady(false);

    SubConfigView::initParentView(view);

    viewconnections << connect(m_latteView, &Latte::View::layoutChanged, this, [this]() {
        if (m_latteView->layout()) {
            updateAvailableScreenGeometry();
        }
    });

    viewconnections << connect(m_latteView, &Latte::View::editThicknessChanged, this, [this]() {
        updateAvailableScreenGeometry();
    });

    viewconnections << connect(m_latteView, &Latte::View::maxNormalThicknessChanged, this, [this]() {
        updateAvailableScreenGeometry();
    });

    viewconnections << connect(m_latteView, &Latte::View::locationChanged, this, [this]() {
        updateAvailableScreenGeometry();
    });

    viewconnections << connect(m_latteView->positioner(), &Latte::ViewPart::Positioner::currentScreenChanged, this, [this]() {
        updateAvailableScreenGeometry();
    });

    viewconnections << connect(m_corona->universalSettings(), &Latte::UniversalSettings::inAdvancedModeForEditSettingsChanged, m_latteView, &Latte::View::inSettingsAdvancedModeChanged);
    viewconnections << connect(m_latteView->containment(), &Plasma::Containment::immutabilityChanged, this, &PrimaryConfigView::immutabilityChanged);   

    m_originalByPassWM = m_latteView->byPassWM();
    m_originalMode = m_latteView->visibility()->mode();

    updateEnabledBorders();
    updateAvailableScreenGeometry();
    syncGeometry();

    setIsReady(true);

    if (m_canvasConfigView) {
        m_canvasConfigView->setParentView(view);
    }

    if (m_secConfigView) {
        m_secConfigView->setParentView(view);
    }

    //! inform view about the current settings level
    emit m_latteView->inSettingsAdvancedModeChanged();
}

void PrimaryConfigView::instantUpdateAvailableScreenGeometry()
{
    if (!m_latteView || !m_latteView->positioner()) {
        return;
    }

    int currentScrId = m_latteView->positioner()->currentScreenId();

    QList<Latte::Types::Visibility> ignoreModes{Latte::Types::SidebarOnDemand,Latte::Types::SidebarAutoHide};

    if (m_latteView->visibility() && m_latteView->visibility()->isSidebar()) {
        ignoreModes.removeAll(Latte::Types::SidebarOnDemand);
        ignoreModes.removeAll(Latte::Types::SidebarAutoHide);
    }

    QString activityid = m_latteView->layout()->lastUsedActivity();

    m_availableScreenGeometry = m_corona->availableScreenRectWithCriteria(currentScrId, activityid, ignoreModes, {}, false, true);
    emit availableScreenGeometryChanged();
}

void PrimaryConfigView::updateAvailableScreenGeometry(View *origin)
{    
    if (!m_latteView || !m_latteView->layout() || m_latteView == origin) {
        return;
    }

    if (!m_availableScreemGeometryTimer.isActive()) {
        m_availableScreemGeometryTimer.start();
    }
}

QRect PrimaryConfigView::availableScreenGeometry() const
{
    return m_availableScreenGeometry;
}

QRect PrimaryConfigView::geometryWhenVisible() const
{
    return m_geometryWhenVisible;
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
    const auto canvasGeometry = m_latteView->positioner()->canvasGeometry();

    int canvasThickness = m_latteView->formFactor() == Plasma::Types::Vertical ? canvasGeometry.width() : canvasGeometry.height();

    QPoint position{0, 0};

    int xPos{0};
    int yPos{0};

    switch (m_latteView->formFactor()) {
    case Plasma::Types::Horizontal: {
        if (inAdvancedMode()) {
            if (qApp->isLeftToRight()) {
                xPos = availGeometry.x() + availGeometry.width() - size.width();
            } else {
                xPos = availGeometry.x();
            }
        } else {
            xPos = scrGeometry.center().x() - size.width() / 2;
        }

        if (location == Plasma::Types::TopEdge) {
            yPos = scrGeometry.y() + canvasThickness;
        } else if (location == Plasma::Types::BottomEdge) {
            yPos = scrGeometry.y() + scrGeometry.height() - canvasThickness - size.height();
        }
    }
        break;

    case Plasma::Types::Vertical: {
        if (location == Plasma::Types::LeftEdge) {
            xPos = scrGeometry.x() + canvasThickness;
            yPos =  availGeometry.y() + (availGeometry.height() - size.height())/2;
        } else if (location == Plasma::Types::RightEdge) {
            xPos = scrGeometry.x() + scrGeometry.width() - canvasThickness - size.width();
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

    QRect winGeometry(x(), y(), width(), height());

    if (m_geometryWhenVisible == geometry && winGeometry == geometry) {
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

    emit m_latteView->configWindowGeometryChanged();
}

void PrimaryConfigView::showEvent(QShowEvent *ev)
{
    updateAvailableScreenGeometry();

    if (m_shellSurface) {
        //! under wayland it needs to be set again after its hiding
        m_shellSurface->setPosition(m_geometryWhenVisible.topLeft());
    }

    SubConfigView::showEvent(ev);

    if (!m_latteView) {
        return;
    }

    setFlags(wFlags());
    m_corona->wm()->setViewExtraFlags(this, false, Latte::Types::NormalWindow);

    syncGeometry();

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &PrimaryConfigView::syncGeometry);

    updateShowInlineProperties();

    showCanvasWindow();

    emit showSignal();

    if (m_latteView && m_latteView->layout()) {
        m_latteView->layout()->setLastConfigViewFor(m_latteView);
    }
}

void PrimaryConfigView::hideEvent(QHideEvent *ev)
{
    if (!m_latteView) {
        return;
    }

    if (m_latteView->containment()) {
        m_latteView->containment()->setUserConfiguring(false);
    }

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

    setVisible(false);
}

bool PrimaryConfigView::hasFocus() const
{
    bool primaryHasHocus{isActive()};
    bool secHasFocus{m_secConfigView && m_secConfigView->isActive()};
    bool canvasHasFocus{m_canvasConfigView && m_canvasConfigView->isActive()};
    bool viewHasFocus{m_latteView && (m_latteView->containsMouse() || m_latteView->alternativesIsShown())};

    return (m_blockFocusLost || viewHasFocus || primaryHasHocus || secHasFocus || canvasHasFocus);
}

void PrimaryConfigView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);

    if (!m_latteView) {
        return;
    }

    const auto *focusWindow = qGuiApp->focusWindow();

    if (focusWindow && (focusWindow->flags().testFlag(Qt::Popup)
                                || focusWindow->flags().testFlag(Qt::ToolTip))) {
        return;
    }

    if (!hasFocus()) {
        hideConfigWindow();
    }
}

void PrimaryConfigView::immutabilityChanged(Plasma::Types::ImmutabilityType type)
{
    if (type != Plasma::Types::Mutable && isVisible()) {
        hideConfigWindow();
    }
}

bool PrimaryConfigView::isReady() const
{
    return m_isReady;
}

void PrimaryConfigView::setIsReady(bool ready)
{
    if (m_isReady == ready) {
        return;
    }

    m_isReady = ready;
    emit isReadyChanged();
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

    if (inAdvancedMode() && m_latteView->formFactor() != Plasma::Types::Vertical) {
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

bool PrimaryConfigView::inAdvancedMode() const
{
    return m_corona->universalSettings()->inAdvancedModeForEditSettings();
}

//!BEGIN borders
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

}
}

