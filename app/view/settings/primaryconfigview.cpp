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
#include "canvasconfigview.h"
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

namespace Latte {
namespace ViewPart {

PrimaryConfigView::PrimaryConfigView(Latte::View *view)
    : SubConfigView(view, QString("#primaryconfigview#"))
{
    connect(this, &QQuickView::widthChanged, this, &PrimaryConfigView::updateEffects);
    connect(this, &QQuickView::heightChanged, this, &PrimaryConfigView::updateEffects);

    connect(this, &PrimaryConfigView::availableScreenGeometryChanged, this, &PrimaryConfigView::syncGeometry);
    connect(this, &PrimaryConfigView::inAdvancedModeChanged, this, &PrimaryConfigView::saveConfig);
    connect(this, &PrimaryConfigView::inAdvancedModeChanged, this, &PrimaryConfigView::updateShowInlineProperties);
    connect(this, &PrimaryConfigView::inAdvancedModeChanged, this, &PrimaryConfigView::syncGeometry);

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
    }

    setParentView(view);
    init();
}

PrimaryConfigView::~PrimaryConfigView()
{
    if (m_secConfigView) {
        delete m_secConfigView;
    }

    if (m_latteView && m_latteView->indicator()) {
        //! destroy indicator config ui when the configuration window is closed
        m_latteView->indicator()->releaseConfigUi();
    }
}

void PrimaryConfigView::init()
{
    SubConfigView::init();

    QByteArray tempFilePath = "lattedockconfigurationui";

    auto source = QUrl::fromLocalFile(m_latteView->containment()->corona()->kPackage().filePath(tempFilePath));
    setSource(source);
    syncGeometry();
    syncSlideEffect();
}

void PrimaryConfigView::setOnActivities(QStringList activities)
{
    m_corona->wm()->setWindowOnActivities(*this, activities);

    if (m_secConfigView) {
        m_corona->wm()->setWindowOnActivities(*m_secConfigView.data(), activities);
    }

    if (m_canvasConfigView) {
        m_corona->wm()->setWindowOnActivities(*m_canvasConfigView.data(), activities);
    }
}

void PrimaryConfigView::requestActivate()
{
    if (m_canvasConfigView) {
        m_canvasConfigView->requestActivate();
    }

    if (m_secConfigView) {
        m_secConfigView->requestActivate();
    }

    SubConfigView::requestActivate();
}

void PrimaryConfigView::showCanvasWindow()
{
    if (!m_canvasConfigView) {
        m_canvasConfigView = new CanvasConfigView(m_latteView, this);
    } else if (m_canvasConfigView && !m_canvasConfigView->isVisible()){
        m_canvasConfigView->show();
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
    if (!m_secConfigView) {
        m_secConfigView = new SecondaryConfigView(m_latteView, this);
    } else if (m_secConfigView && !m_secConfigView->isVisible()){
        m_secConfigView->show();
    }
}

void PrimaryConfigView::hideSecondaryWindow()
{
    if (m_secConfigView) {
        m_secConfigView->hideConfigWindow();

        if (KWindowSystem::isPlatformX11() && m_latteView->effects()) {
            //! this is needed in order for subtracked mask of secondary window to
            //! be released properly when changing for Advanced to Basic mode.
            //! Under wayland this is not needed because masks do not break any visuals.
            m_latteView->effects()->updateMask();
        }
    }
}

void PrimaryConfigView::setParentView(Latte::View *view)
{
    if (m_latteView == view) {
        return;
    }

    if (m_latteView) {
        hideConfigWindow();

        QTimer::singleShot(400, [this, view]() {
            initParentView(view);
        });
    } else {
        initParentView(view);
    }
}

void PrimaryConfigView::initParentView(Latte::View *view)
{
    setIsReady(false);

    if (m_latteView && m_latteView->indicator()) {
        //! destroy indicator config ui when the configuration window is closed
        m_latteView->indicator()->releaseConfigUi();
    }

    SubConfigView::initParentView(view);

    viewconnections << connect(this, &PrimaryConfigView::inAdvancedModeChanged, m_latteView, &Latte::View::inSettingsAdvancedModeChanged);
    viewconnections << connect(m_latteView->containment(), &Plasma::Containment::immutabilityChanged, this, &PrimaryConfigView::immutabilityChanged);

    m_originalByPassWM = m_latteView->byPassWM();
    m_originalMode = m_latteView->visibility()->mode();

    loadConfig();

    //! inform view about the current settings level
    emit m_latteView->inSettingsAdvancedModeChanged();

    updateEnabledBorders();
    updateAvailableScreenGeometry();
    syncGeometry();

    show();
    showCanvasWindow();

    setIsReady(true);

    if (m_canvasConfigView) {
        m_canvasConfigView->setParentView(view);
    }

    if (m_secConfigView) {
        m_secConfigView->setParentView(view);
    }
}

void PrimaryConfigView::updateAvailableScreenGeometry(View *origin)
{    
    if (!m_latteView) {
        return;
    }

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

    showCanvasWindow();

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

    setVisible(false);
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

    hideCanvasWindow();
    hideSecondaryWindow();
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

