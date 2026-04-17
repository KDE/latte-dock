/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-FileCopyrightText: 2026 OpenAI
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "waylandinterface.h"

// local
#include "waylandsurface.h"
#include <coretypes.h>
#include "../infoview.h"
#include "../lattecorona.h"
#include "../view/helpers/screenedgeghostwindow.h"
#include "../view/helpers/subwindow.h"
#include "../view/positioner.h"
#include "../view/settings/subconfigview.h"
#include "../view/view.h"

// Qt
#include <QApplication>
#include <QDebug>
#include <QQuickView>
#include <QSet>
#include <QTimer>
#include <utility>

// KDE
#include <KWindowEffects>
#include <KWindowSystem>
#include <LayerShellQt/Window>
#include <taskmanager/abstracttasksmodel.h>
#include <taskmanager/virtualdesktopinfo.h>
#include <taskmanager/windowtasksmodel.h>

// X11
#include <NETWM>

namespace {

using TaskRoles = TaskManager::AbstractTasksModel;

static QStringList toStringList(const QVariant &value)
{
    if (value.canConvert<QStringList>()) {
        return value.toStringList();
    }

    QStringList result;
    const QVariantList list = value.toList();

    for (const QVariant &entry : list) {
        result << entry.toString();
    }

    return result;
}

static QList<Latte::WindowSystem::WindowId> toWindowIds(const QVariant &value)
{
    QList<Latte::WindowSystem::WindowId> ids;
    const QVariantList list = value.toList();

    for (const QVariant &entry : list) {
        ids << Latte::WindowSystem::WindowId(entry);
    }

    return ids;
}

static bool appIdMatches(const QString &candidate, const QString &expected)
{
    if (candidate == expected) {
        return true;
    }

    QString normalizedCandidate = candidate;
    QString normalizedExpected = expected;

    if (normalizedCandidate.endsWith(QLatin1String(".desktop"))) {
        normalizedCandidate.chop(8);
    }

    if (normalizedExpected.endsWith(QLatin1String(".desktop"))) {
        normalizedExpected.chop(8);
    }

    return normalizedCandidate == normalizedExpected
            || normalizedCandidate.endsWith(QLatin1Char('.') + normalizedExpected)
            || normalizedExpected.endsWith(QLatin1Char('.') + normalizedCandidate);
}

}

namespace Latte {

class Private::GhostWindow : public QQuickView
{
    Q_OBJECT

public:
    WindowSystem::WindowId m_winId;

    explicit GhostWindow(WindowSystem::WaylandInterface *waylandInterface)
        : m_waylandInterface(waylandInterface),
          m_surface(new WindowSystem::WaylandSurface(this, this))
    {
        setFlags(Qt::FramelessWindowHint
                 | Qt::WindowStaysOnTopHint
                 | Qt::NoDropShadowWindowHint
                 | Qt::WindowDoesNotAcceptFocus);

        setColor(QColor(Qt::transparent));

        connect(m_waylandInterface, &WindowSystem::AbstractWindowInterface::latteWindowAdded, this, &GhostWindow::identifyWinId);

        show();
        setupWaylandIntegration();
    }

    ~GhostWindow() override
    {
        m_waylandInterface->unregisterIgnoredWindow(m_winId);
    }

    void setGeometry(const QRect &rect)
    {
        if (geometry() == rect) {
            return;
        }

        m_validGeometry = rect;

        setMinimumSize(rect.size());
        setMaximumSize(rect.size());
        resize(rect.size());

        if (m_surface) {
            m_surface->setPosition(rect.topLeft());
        }
    }

    void setupWaylandIntegration()
    {
        if (!m_surface || !m_surface->sync()) {
            return;
        }

        m_surface->setSkipTaskbar(true);
        m_surface->setPanelTakesFocus(false);
        m_surface->setRole(WindowSystem::WaylandSurface::Role::Panel);
        m_surface->setPanelBehavior(WindowSystem::WaylandSurface::PanelBehavior::AlwaysVisible);
    }

public slots:
    void identifyWinId()
    {
        if (m_winId.isNull()) {
            m_winId = m_waylandInterface->winIdFor(QStringLiteral("latte-dock"), m_validGeometry);
            m_waylandInterface->registerIgnoredWindow(m_winId);
        }
    }

private:
    WindowSystem::WaylandInterface *m_waylandInterface{nullptr};
    WindowSystem::WaylandSurface *m_surface{nullptr};
    QRect m_validGeometry;
};

namespace WindowSystem {

WaylandInterface::WaylandInterface(QObject *parent)
    : AbstractWindowInterface(parent)
{
    m_corona = qobject_cast<Latte::Corona *>(parent);
}

WaylandInterface::~WaylandInterface() = default;

void WaylandInterface::init()
{
    if (m_windowTasksModel) {
        return;
    }

    m_windowTasksModel = new TaskManager::WindowTasksModel(this);
    m_virtualDesktopInfo = new TaskManager::VirtualDesktopInfo(this);

    connect(m_windowTasksModel, &QAbstractItemModel::modelReset, this, [this]() {
        rebuildWindowIndexCache();
        refreshModelState();
    });

    connect(m_windowTasksModel, &QAbstractItemModel::rowsInserted, this, [this]() {
        rebuildWindowIndexCache();
        refreshModelState();
    });

    connect(m_windowTasksModel, &QAbstractItemModel::rowsRemoved, this, [this]() {
        rebuildWindowIndexCache();
        refreshModelState();
    });

    connect(m_windowTasksModel, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &) {
        QSet<WindowId> changedIds;

        for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
            const QModelIndex index = m_windowTasksModel->index(row, 0, topLeft.parent());
            const WindowId wid = windowIdForIndex(index);

            if (!wid.isNull()) {
                changedIds.insert(wid);
            }
        }

        rebuildWindowIndexCache();
        refreshModelState(changedIds);
    });

    connect(m_virtualDesktopInfo, &TaskManager::VirtualDesktopInfo::currentDesktopChanged, this, [this]() {
        setCurrentDesktop(m_virtualDesktopInfo->currentDesktop().toString());
    });

    connect(m_virtualDesktopInfo, &TaskManager::VirtualDesktopInfo::desktopIdsChanged, this, [this]() {
        syncDesktops();
    });

    syncDesktops();
    setCurrentDesktop(m_virtualDesktopInfo->currentDesktop().toString());
    rebuildWindowIndexCache();
    refreshModelState();
}

void WaylandInterface::initWindowManagement()
{
    init();
}

void WaylandInterface::rebuildWindowIndexCache()
{
    m_windowIndexes.clear();

    if (!m_windowTasksModel) {
        return;
    }

    for (int row = 0; row < m_windowTasksModel->rowCount(); ++row) {
        const QModelIndex index = m_windowTasksModel->index(row, 0);
        const WindowId wid = windowIdForIndex(index);

        if (!wid.isNull()) {
            m_windowIndexes.insert(wid, index);
        }
    }
}

void WaylandInterface::refreshModelState(const QSet<WindowId> &changedIds)
{
    QSet<WindowId> nextTrackedWindows;

    for (auto it = m_windowIndexes.constBegin(); it != m_windowIndexes.constEnd(); ++it) {
        if (isAcceptableWindow(it.value())) {
            nextTrackedWindows.insert(it.key());
        }
    }

    for (const WindowId &wid : std::as_const(m_trackedWindows)) {
        if (!nextTrackedWindows.contains(wid)) {
            emit windowRemoved(wid);
        }
    }

    for (const WindowId &wid : std::as_const(nextTrackedWindows)) {
        if (!m_trackedWindows.contains(wid)) {
            emit windowAdded(wid);

            if (appIdMatches(indexFor(wid), QStringLiteral("latte-dock"))) {
                emit latteWindowAdded();
            }
        } else if (changedIds.contains(wid)) {
            considerWindowChanged(wid);
        }
    }

    m_trackedWindows = nextTrackedWindows;

    const WindowId nextActiveWindow = activeWindowFromModel();

    if (m_activeWindowId != nextActiveWindow) {
        m_activeWindowId = nextActiveWindow;
        emit activeWindowChanged(m_activeWindowId);
    }
}

QModelIndex WaylandInterface::indexFor(WindowId wid) const
{
    const auto it = m_windowIndexes.constFind(wid);

    if (it == m_windowIndexes.constEnd() || !it.value().isValid()) {
        return {};
    }

    return it.value();
}

WindowId WaylandInterface::windowIdForIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return WindowId::nil();
    }

    const QList<WindowId> ids = toWindowIds(index.data(TaskRoles::WinIdList));
    return ids.isEmpty() ? WindowId::nil() : ids.constFirst();
}

WindowId WaylandInterface::activeWindowFromModel() const
{
    for (auto it = m_windowIndexes.constBegin(); it != m_windowIndexes.constEnd(); ++it) {
        if (it.value().data(TaskRoles::IsActive).toBool() && !hasBlockedTracking(it.key())) {
            return it.key();
        }
    }

    return WindowId::nil();
}

WindowInfoWrap WaylandInterface::requestInfo(const QModelIndex &index) const
{
    WindowInfoWrap winfo;

    if (!index.isValid()) {
        return winfo;
    }

    const WindowId wid = windowIdForIndex(index);
    winfo.setWid(wid);
    winfo.setParentId(WindowId::nil());
    winfo.setIsValid(isValidWindow(index));
    winfo.setIsActive(index.data(TaskRoles::IsActive).toBool());
    winfo.setIsMinimized(index.data(TaskRoles::IsMinimized).toBool());
    winfo.setIsMaxVert(index.data(TaskRoles::IsMaximized).toBool());
    winfo.setIsMaxHoriz(index.data(TaskRoles::IsMaximized).toBool());
    winfo.setIsFullscreen(index.data(TaskRoles::IsFullScreen).toBool());
    winfo.setIsShaded(index.data(TaskRoles::IsShaded).toBool());
    winfo.setIsOnAllDesktops(index.data(TaskRoles::IsOnAllVirtualDesktops).toBool());
    winfo.setIsOnAllActivities(toStringList(index.data(TaskRoles::Activities)).isEmpty());
    winfo.setIsKeepAbove(index.data(TaskRoles::IsKeepAbove).toBool());
    winfo.setIsKeepBelow(index.data(TaskRoles::IsKeepBelow).toBool());
    winfo.setGeometry(index.data(TaskRoles::Geometry).toRect());
    winfo.setHasSkipPager(index.data(TaskRoles::SkipPager).toBool());
    winfo.setHasSkipSwitcher(false);
    winfo.setHasSkipTaskbar(index.data(TaskRoles::SkipTaskbar).toBool());

    winfo.setIsClosable(index.data(TaskRoles::IsClosable).toBool());
    winfo.setIsFullScreenable(index.data(TaskRoles::IsFullScreenable).toBool());
    winfo.setIsGroupable(index.data(TaskRoles::IsGroupable).toBool());
    winfo.setIsMaximizable(index.data(TaskRoles::IsMaximizable).toBool());
    winfo.setIsMinimizable(index.data(TaskRoles::IsMinimizable).toBool());
    winfo.setIsMovable(index.data(TaskRoles::IsMovable).toBool());
    winfo.setIsResizable(index.data(TaskRoles::IsResizable).toBool());
    winfo.setIsShadeable(index.data(TaskRoles::IsShadeable).toBool());
    winfo.setIsVirtualDesktopsChangeable(index.data(TaskRoles::IsVirtualDesktopsChangeable).toBool());

    winfo.setDisplay(index.data(Qt::DisplayRole).toString());
    winfo.setIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
    winfo.setDesktops(toStringList(index.data(TaskRoles::VirtualDesktops)));
    winfo.setActivities(toStringList(index.data(TaskRoles::Activities)));

    return winfo;
}

bool WaylandInterface::appIdMatches(const QModelIndex &index, const QString &expected) const
{
    return ::appIdMatches(index.data(TaskRoles::AppId).toString(), expected);
}

bool WaylandInterface::isPlasmaPanel(const QModelIndex &index) const
{
    if (!index.isValid() || !appIdMatches(index, QStringLiteral("org.kde.plasmashell"))) {
        return false;
    }

    return AbstractWindowInterface::isPlasmaPanel(index.data(TaskRoles::Geometry).toRect());
}

bool WaylandInterface::isFullScreenWindow(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }

    return index.data(TaskRoles::IsFullScreen).toBool()
            || AbstractWindowInterface::isFullScreenWindow(index.data(TaskRoles::Geometry).toRect());
}

bool WaylandInterface::isSidepanel(const QModelIndex &index) const
{
    return index.isValid() && AbstractWindowInterface::isSidepanel(index.data(TaskRoles::Geometry).toRect());
}

bool WaylandInterface::isValidWindow(const QModelIndex &index) const
{
    const WindowId wid = windowIdForIndex(index);

    if (wid.isNull()) {
        return false;
    }

    if (windowsTracker()->isValidFor(wid)) {
        return true;
    }

    return isAcceptableWindow(index);
}

bool WaylandInterface::isAcceptableWindow(const QModelIndex &index) const
{
    const WindowId wid = windowIdForIndex(index);

    if (wid.isNull()) {
        return false;
    }

    if (hasBlockedTracking(wid)) {
        return false;
    }

    if (isWhitelistedWindow(wid)) {
        return true;
    }

    const bool hasSkipTaskbar = index.data(TaskRoles::SkipTaskbar).toBool();
    const bool hasSkipSwitcher = false;
    const bool isSkipped = hasSkipTaskbar && (hasSkipSwitcher || hasSkipTaskbar);

    if (isSkipped && (appIdMatches(index, QStringLiteral("yakuake")) || appIdMatches(index, QStringLiteral("krunner")))) {
        const_cast<WaylandInterface *>(this)->registerWhitelistedWindow(wid);
    } else if (appIdMatches(index, QStringLiteral("org.kde.plasmashell"))) {
        if (isSkipped && isSidepanel(index)) {
            const_cast<WaylandInterface *>(this)->registerWhitelistedWindow(wid);
            return true;
        } else if (isPlasmaPanel(index) || isFullScreenWindow(index)) {
            const_cast<WaylandInterface *>(this)->registerPlasmaIgnoredWindow(wid);
            return false;
        }
    } else if (appIdMatches(index, QStringLiteral("latte-dock")) || appIdMatches(index, QStringLiteral("org.kde.latte-dock"))
               || appIdMatches(index, QStringLiteral("ksmserver"))) {
        if (isFullScreenWindow(index)) {
            const_cast<WaylandInterface *>(this)->registerIgnoredWindow(wid);
            return false;
        }
    }

    return !hasSkipTaskbar;
}

void WaylandInterface::setCurrentDesktop(const QString &desktop)
{
    if (m_currentDesktop == desktop) {
        return;
    }

    m_currentDesktop = desktop;
    emit currentDesktopChanged();
}

void WaylandInterface::syncDesktops()
{
    if (!m_virtualDesktopInfo) {
        m_desktops.clear();
        return;
    }

    m_desktops = toStringList(m_virtualDesktopInfo->desktopIds());
}

void WaylandInterface::registerIgnoredWindow(WindowId wid)
{
    if (wid.isNull() || m_ignoredWindows.contains(wid)) {
        return;
    }

    m_ignoredWindows.append(wid);

    if (m_trackedWindows.remove(wid)) {
        emit windowRemoved(wid);
    } else {
        emit windowChanged(wid);
    }
}

void WaylandInterface::unregisterIgnoredWindow(WindowId wid)
{
    if (!m_ignoredWindows.contains(wid)) {
        return;
    }

    m_ignoredWindows.removeAll(wid);
    emit windowRemoved(wid);
    refreshModelState({wid});
}

void WaylandInterface::setViewExtraFlags(QObject *view, bool isPanelWindow, Latte::Types::Visibility mode)
{
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(view);
    Latte::View *latteView = qobject_cast<Latte::View *>(view);
    Latte::ViewPart::SubConfigView *configView = qobject_cast<Latte::ViewPart::SubConfigView *>(view);
    Latte::ViewPart::SubWindow *subWindow = qobject_cast<Latte::ViewPart::SubWindow *>(view);
    Latte::InfoView *infoView = qobject_cast<Latte::InfoView *>(view);

    WindowId winId;

    if (latteView) {
        surface = latteView->surface();
        winId = latteView->positioner()->trackedWindowId();
    } else if (configView) {
        surface = configView->surface();
        winId = configView->trackedWindowId();
    } else if (subWindow) {
        surface = subWindow->surface();
        winId = subWindow->trackedWindowId();
    } else if (infoView) {
        surface = infoView->waylandSurface();
    }

    if (!surface) {
        return;
    }

    const bool canCover = (mode == Latte::Types::WindowsCanCover || mode == Latte::Types::WindowsAlwaysCover);

    surface->setSkipTaskbar(true);
    surface->setSkipSwitcher(true);

    const bool atBottom = !isPanelWindow && canCover;

    if (isPanelWindow) {
        surface->setRole(WaylandSurface::Role::Panel);
        surface->setPanelBehavior(WaylandSurface::PanelBehavior::AutoHide);
    } else {
        surface->setRole(WaylandSurface::Role::Normal);
    }

    if (!winId.isNull()) {
        const WindowInfoWrap winfo = requestInfo(winId);

        if (winfo.isValid() && !winfo.isOnAllDesktops()) {
            requestToggleIsOnAllDesktops(winId);
        }

        if (mode == Latte::Types::WindowsCanCover || mode == Latte::Types::WindowsAlwaysCover) {
            setKeepBelow(winId, true);
        } else if (mode == Latte::Types::NormalWindow) {
            setKeepBelow(winId, false);
            setKeepAbove(winId, false);
        } else {
            setKeepAbove(winId, true);
        }
    }

    if (latteView) {
        if (auto *layerWindow = latteView->layerWindow()) {
            layerWindow->setScreen(latteView->screen());
            layerWindow->setLayer(canCover ? LayerShellQt::Window::LayerBottom
                                           : LayerShellQt::Window::LayerTop);
            layerWindow->setKeyboardInteractivity(latteView->flags().testFlag(Qt::WindowDoesNotAcceptFocus)
                                                  ? LayerShellQt::Window::KeyboardInteractivityNone
                                                  : LayerShellQt::Window::KeyboardInteractivityOnDemand);
            layerWindow->setExclusiveZone((!canCover && isPanelWindow)
                                          ? (latteView->formFactor() == Plasma::Types::Horizontal
                                             ? latteView->height()
                                             : latteView->width())
                                          : 0);
        }
    }

    if (atBottom) {
        QPointer<WaylandSurface> safeSurface(surface);
        QTimer::singleShot(50, this, [safeSurface]() {
            if (safeSurface) {
                safeSurface->setRole(WaylandSurface::Role::ToolTip);
            }
        });
    }
}

void WaylandInterface::setViewStruts(QWindow &view, const QRect &rect, Plasma::Types::Location location)
{
    if (!m_ghostWindows.contains(view.winId())) {
        m_ghostWindows[view.winId()] = new Private::GhostWindow(this);
    }

    auto w = m_ghostWindows[view.winId()];

    switch (location) {
    case Plasma::Types::TopEdge:
    case Plasma::Types::BottomEdge:
        w->setGeometry({rect.x() + rect.width() / 2 - rect.height(), rect.y(), rect.height() + 1, rect.height()});
        break;

    case Plasma::Types::LeftEdge:
    case Plasma::Types::RightEdge:
        w->setGeometry({rect.x(), rect.y() + rect.height() / 2 - rect.width(), rect.width(), rect.width() + 1});
        break;

    default:
        break;
    }
}

void WaylandInterface::switchToNextVirtualDesktop()
{
    if (!m_virtualDesktopInfo || m_desktops.count() <= 1) {
        return;
    }

    int curPos = m_desktops.indexOf(m_currentDesktop);
    int nextPos = curPos + 1;

    if (curPos >= m_desktops.count() - 1) {
        if (isVirtualDesktopNavigationWrappingAround()) {
            nextPos = 0;
        } else {
            return;
        }
    }

    m_virtualDesktopInfo->requestActivate(m_desktops[nextPos]);
}

void WaylandInterface::switchToPreviousVirtualDesktop()
{
    if (!m_virtualDesktopInfo || m_desktops.count() <= 1) {
        return;
    }

    int curPos = m_desktops.indexOf(m_currentDesktop);
    int nextPos = curPos - 1;

    if (curPos <= 0) {
        if (isVirtualDesktopNavigationWrappingAround()) {
            nextPos = m_desktops.count() - 1;
        } else {
            return;
        }
    }

    m_virtualDesktopInfo->requestActivate(m_desktops[nextPos]);
}

void WaylandInterface::setWindowOnActivities(const WindowId &wid, const QStringList &nextactivities)
{
    const QModelIndex index = indexFor(wid);

    if (!index.isValid() || !m_windowTasksModel) {
        return;
    }

    m_windowTasksModel->requestActivities(index, nextactivities);
}

void WaylandInterface::removeViewStruts(QWindow &view)
{
    delete m_ghostWindows.take(view.winId());
}

WindowId WaylandInterface::activeWindow()
{
    return m_activeWindowId.isNull() ? activeWindowFromModel() : m_activeWindowId;
}

void WaylandInterface::skipTaskBar(const QDialog &dialog)
{
    Q_UNUSED(dialog);
}

void WaylandInterface::slideWindow(QWindow &view, AbstractWindowInterface::Slide location)
{
    auto slideLocation = KWindowEffects::NoEdge;

    switch (location) {
    case Slide::Top:
        slideLocation = KWindowEffects::TopEdge;
        break;
    case Slide::Bottom:
        slideLocation = KWindowEffects::BottomEdge;
        break;
    case Slide::Left:
        slideLocation = KWindowEffects::LeftEdge;
        break;
    case Slide::Right:
        slideLocation = KWindowEffects::RightEdge;
        break;
    default:
        break;
    }

    KWindowEffects::slideWindow(&view, slideLocation, -1);
}

void WaylandInterface::enableBlurBehind(QWindow &view)
{
    KWindowEffects::enableBlurBehind(&view);
}

void WaylandInterface::setActiveEdge(QWindow *view, bool active)
{
    auto *window = qobject_cast<ViewPart::ScreenEdgeGhostWindow *>(view);

    if (!window) {
        return;
    }

    if (window->parentView()->surface() && window->parentView()->visibility()
            && (window->parentView()->visibility()->mode() == Types::DodgeActive
                || window->parentView()->visibility()->mode() == Types::DodgeMaximized
                || window->parentView()->visibility()->mode() == Types::DodgeAllWindows
                || window->parentView()->visibility()->mode() == Types::AutoHide)) {
        if (active) {
            window->showWithMask();
            window->surface()->requestHideAutoHidingPanel();
        } else {
            window->hideWithMask();
            window->surface()->requestShowAutoHidingPanel();
        }
    }
}

void WaylandInterface::setFrameExtents(QWindow *view, const QMargins &extents)
{
    Q_UNUSED(view);
    Q_UNUSED(extents);
}

void WaylandInterface::setInputMask(QWindow *window, const QRect &rect)
{
    Q_UNUSED(window);
    Q_UNUSED(rect);
}

WindowInfoWrap WaylandInterface::requestInfoActive()
{
    return requestInfo(activeWindow());
}

WindowInfoWrap WaylandInterface::requestInfo(WindowId wid)
{
    const QModelIndex index = indexFor(wid);
    WindowInfoWrap winfo = requestInfo(index);

    const bool plasmaBlockedWindow = index.isValid() && appIdMatches(index, QStringLiteral("org.kde.plasmashell")) && !isAcceptableWindow(index);

    if (plasmaBlockedWindow) {
        windowRemoved(wid);
    }

    return winfo;
}

AppData WaylandInterface::appDataFor(WindowId wid)
{
    const QModelIndex index = indexFor(wid);

    if (!index.isValid()) {
        return {};
    }

    const QUrl launcherUrl = index.data(TaskRoles::LauncherUrlWithoutIcon).toUrl();

    if (launcherUrl.isValid()) {
        return appDataFromUrl(launcherUrl, qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
    }

    return appDataFromUrl(windowUrlFromMetadata(index.data(TaskRoles::AppId).toString(),
                                                index.data(TaskRoles::AppPid).toUInt(),
                                                rulesConfig),
                          qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
}

QIcon WaylandInterface::iconFor(WindowId wid)
{
    return qvariant_cast<QIcon>(indexFor(wid).data(Qt::DecorationRole));
}

WindowId WaylandInterface::winIdFor(QString appId, QString title)
{
    for (auto it = m_windowIndexes.constBegin(); it != m_windowIndexes.constEnd(); ++it) {
        if (appIdMatches(it.value(), appId) && it.value().data(Qt::DisplayRole).toString().startsWith(title)) {
            return it.key();
        }
    }

    return {};
}

WindowId WaylandInterface::winIdFor(QString appId, QRect geometry)
{
    for (auto it = m_windowIndexes.constBegin(); it != m_windowIndexes.constEnd(); ++it) {
        if (appIdMatches(it.value(), appId) && it.value().data(TaskRoles::Geometry).toRect() == geometry) {
            return it.key();
        }
    }

    return {};
}

bool WaylandInterface::windowCanBeDragged(WindowId wid)
{
    const WindowInfoWrap winfo = requestInfo(wid);
    return winfo.isValid() && winfo.isMovable() && !winfo.isMinimized() && inCurrentDesktopActivity(winfo);
}

bool WaylandInterface::windowCanBeMaximized(WindowId wid)
{
    const WindowInfoWrap winfo = requestInfo(wid);
    return winfo.isValid() && winfo.isMaximizable() && !winfo.isMinimized() && inCurrentDesktopActivity(winfo);
}

void WaylandInterface::requestActivate(WindowId wid)
{
    const QModelIndex index = indexFor(wid);

    if (index.isValid() && m_windowTasksModel) {
        m_windowTasksModel->requestActivate(index);
    }
}

void WaylandInterface::requestClose(WindowId wid)
{
    const QModelIndex index = indexFor(wid);

    if (index.isValid() && m_windowTasksModel) {
        m_windowTasksModel->requestClose(index);
    }
}

void WaylandInterface::requestMoveWindow(WindowId wid, QPoint from)
{
    Q_UNUSED(from);

    const QModelIndex index = indexFor(wid);
    const WindowInfoWrap winfo = requestInfo(wid);

    if (index.isValid() && m_windowTasksModel && windowCanBeDragged(wid) && inCurrentDesktopActivity(winfo)) {
        m_windowTasksModel->requestMove(index);
    }
}

void WaylandInterface::requestToggleIsOnAllDesktops(WindowId wid)
{
    const QModelIndex index = indexFor(wid);
    const WindowInfoWrap winfo = requestInfo(wid);

    if (!index.isValid() || !m_windowTasksModel || m_desktops.count() <= 1) {
        return;
    }

    if (winfo.isOnAllDesktops()) {
        m_windowTasksModel->requestVirtualDesktops(index, QVariantList({m_currentDesktop}));
    } else {
        m_windowTasksModel->requestVirtualDesktops(index, {});
    }
}

void WaylandInterface::requestToggleKeepAbove(WindowId wid)
{
    const QModelIndex index = indexFor(wid);

    if (index.isValid() && m_windowTasksModel) {
        m_windowTasksModel->requestToggleKeepAbove(index);
    }
}

void WaylandInterface::setKeepAbove(WindowId wid, bool active)
{
    const QModelIndex index = indexFor(wid);

    if (!index.isValid() || !m_windowTasksModel) {
        return;
    }

    const bool isKeepAbove = index.data(TaskRoles::IsKeepAbove).toBool();

    if (active) {
        setKeepBelow(wid, false);
    }

    if ((isKeepAbove && active) || (!isKeepAbove && !active)) {
        return;
    }

    m_windowTasksModel->requestToggleKeepAbove(index);
}

void WaylandInterface::setKeepBelow(WindowId wid, bool active)
{
    const QModelIndex index = indexFor(wid);

    if (!index.isValid() || !m_windowTasksModel) {
        return;
    }

    const bool isKeepBelow = index.data(TaskRoles::IsKeepBelow).toBool();

    if (active) {
        setKeepAbove(wid, false);
    }

    if ((isKeepBelow && active) || (!isKeepBelow && !active)) {
        return;
    }

    m_windowTasksModel->requestToggleKeepBelow(index);
}

void WaylandInterface::requestToggleMinimized(WindowId wid)
{
    const QModelIndex index = indexFor(wid);
    const WindowInfoWrap winfo = requestInfo(wid);

    if (index.isValid() && m_windowTasksModel && isValidWindow(index) && inCurrentDesktopActivity(winfo)) {
        if (!m_currentDesktop.isEmpty() && !winfo.isOnAllDesktops() && !winfo.isOnDesktop(m_currentDesktop)) {
            m_windowTasksModel->requestVirtualDesktops(index, QVariantList({m_currentDesktop}));
        }

        m_windowTasksModel->requestToggleMinimized(index);
    }
}

void WaylandInterface::requestToggleMaximized(WindowId wid)
{
    const QModelIndex index = indexFor(wid);
    const WindowInfoWrap winfo = requestInfo(wid);

    if (index.isValid() && m_windowTasksModel && windowCanBeMaximized(wid) && inCurrentDesktopActivity(winfo)) {
        if (!m_currentDesktop.isEmpty() && !winfo.isOnAllDesktops() && !winfo.isOnDesktop(m_currentDesktop)) {
            m_windowTasksModel->requestVirtualDesktops(index, QVariantList({m_currentDesktop}));
        }

        m_windowTasksModel->requestToggleMaximized(index);
    }
}

}
}

#include "waylandinterface.moc"
