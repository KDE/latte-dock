/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xwindowinterface.h"

// local
#include <coretypes.h>
#include "tasktools.h"
#include "view/view.h"
#include "view/helpers/screenedgeghostwindow.h"

// Qt
#include <QDebug>
#include <QTimer>
#include <QtX11Extras/QX11Info>

// KDE
#include <KDesktopFile>
#include <KWindowSystem>
#include <KWindowInfo>
#include <KIconThemes/KIconLoader>

// X11
#include <NETWM>
#include <xcb/xcb.h>
#include <xcb/shape.h>

namespace Latte {
namespace WindowSystem {

XWindowInterface::XWindowInterface(QObject *parent)
    : AbstractWindowInterface(parent)
{
    m_currentDesktop = QString(KWindowSystem::self()->currentDesktop());

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &AbstractWindowInterface::activeWindowChanged);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &AbstractWindowInterface::windowRemoved);

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &XWindowInterface::windowAddedProxy);

    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [&](int desktop) {
        m_currentDesktop = QString(desktop);
        emit currentDesktopChanged();
    });

    connect(KWindowSystem::self()
            , static_cast<void (KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>
            (&KWindowSystem::windowChanged)
            , this, &XWindowInterface::windowChangedProxy);


    for(auto wid : KWindowSystem::self()->windows()) {
        windowAddedProxy(wid);
    }
}

XWindowInterface::~XWindowInterface()
{
}

void XWindowInterface::setViewExtraFlags(QObject *view,bool isPanelWindow, Latte::Types::Visibility mode)
{
    WId winId = -1;

    QQuickView *quickView = qobject_cast<QQuickView *>(view);

    if (quickView) {
        winId = quickView->winId();
    }

    if (!quickView) {
        QQuickWindow *quickWindow = qobject_cast<QQuickWindow *>(view);

        if (quickWindow) {
            winId = quickWindow->winId();
        }
    }

    NETWinInfo winfo(QX11Info::connection()
                     , static_cast<xcb_window_t>(winId)
                     , static_cast<xcb_window_t>(winId)
                     , 0, 0);

    winfo.setAllowedActions(NET::ActionChangeDesktop);

    if (isPanelWindow) {
        KWindowSystem::setType(winId, NET::Dock);
    } else {
        KWindowSystem::setType(winId, NET::Normal);
    }

    KWindowSystem::setState(winId, NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    KWindowSystem::setOnAllDesktops(winId, true);

    //! Layer to be applied
    if (mode == Latte::Types::WindowsCanCover || mode == Latte::Types::WindowsAlwaysCover) {
        setKeepBelow(winId, true);
    } else if (mode == Latte::Types::NormalWindow) {
        setKeepBelow(winId, false);
        setKeepAbove(winId, false);
    } else {
        setKeepAbove(winId, true);
    }
}

void XWindowInterface::setViewStruts(QWindow &view, const QRect &rect
                                     , Plasma::Types::Location location)
{
    NETExtendedStrut strut;

    const auto screen = view.screen();

    const QRect currentScreen {screen->geometry()};
    const QRect wholeScreen {{0, 0}, screen->virtualSize()};

    switch (location) {
    case Plasma::Types::TopEdge: {
        const int topOffset {screen->geometry().top()};
        strut.top_width = rect.height() + topOffset;
        strut.top_start = rect.x();
        strut.top_end = rect.x() + rect.width() - 1;
        break;
    }

    case Plasma::Types::BottomEdge: {
        const int bottomOffset {wholeScreen.bottom() - currentScreen.bottom()};
        strut.bottom_width = rect.height() + bottomOffset;
        strut.bottom_start = rect.x();
        strut.bottom_end = rect.x() + rect.width() - 1;
        break;
    }

    case Plasma::Types::LeftEdge: {
        const int leftOffset = {screen->geometry().left()};
        strut.left_width = rect.width() + leftOffset;
        strut.left_start = rect.y();
        strut.left_end = rect.y() + rect.height() - 1;
        break;
    }

    case Plasma::Types::RightEdge: {
        const int rightOffset = {wholeScreen.right() - currentScreen.right()};
        strut.right_width = rect.width() + rightOffset;
        strut.right_start = rect.y();
        strut.right_end = rect.y() + rect.height() - 1;
        break;
    }

    default:
        qWarning() << "wrong location:" << location;
        return;
    }

    KWindowSystem::setExtendedStrut(view.winId(),
                                    strut.left_width,   strut.left_start,   strut.left_end,
                                    strut.right_width,  strut.right_start,  strut.right_end,
                                    strut.top_width,    strut.top_start,    strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end
                                    );
}

void XWindowInterface::switchToNextVirtualDesktop()
{
    int desktops = KWindowSystem::numberOfDesktops();

    if (desktops <= 1) {
        return;
    }

    int curPos = KWindowSystem::currentDesktop();
    int nextPos = curPos + 1;

    if (curPos == desktops) {
        if (isVirtualDesktopNavigationWrappingAround()) {
            nextPos = 1;
        } else {
            return;
        }
    }

    KWindowSystem::setCurrentDesktop(nextPos);
}

void XWindowInterface::switchToPreviousVirtualDesktop()
{
    int desktops = KWindowSystem::numberOfDesktops();
    if (desktops <= 1) {
        return;
    }

    int curPos = KWindowSystem::currentDesktop();
    int nextPos = curPos - 1;

    if (curPos == 1) {
        if (isVirtualDesktopNavigationWrappingAround()) {
            nextPos = desktops;
        } else {
            return;
        }
    }

    KWindowSystem::setCurrentDesktop(nextPos);
}

void XWindowInterface::setWindowOnActivities(const WindowId &wid, const QStringList &activities)
{
    KWindowSystem::setOnActivities(wid.toUInt(), activities);
}

void XWindowInterface::removeViewStruts(QWindow &view)
{
    KWindowSystem::setStrut(view.winId(), 0, 0, 0, 0);
}

WindowId XWindowInterface::activeWindow()
{
    return KWindowSystem::self()->activeWindow();
}

void XWindowInterface::skipTaskBar(const QDialog &dialog)
{
    KWindowSystem::setState(dialog.winId(), NET::SkipTaskbar);
}

void XWindowInterface::slideWindow(QWindow &view, AbstractWindowInterface::Slide location)
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

    KWindowEffects::slideWindow(view.winId(), slideLocation, -1);
}

void XWindowInterface::enableBlurBehind(QWindow &view)
{
    KWindowEffects::enableBlurBehind(view.winId());
}

void XWindowInterface::setActiveEdge(QWindow *view, bool active)
{
    ViewPart::ScreenEdgeGhostWindow *window = qobject_cast<ViewPart::ScreenEdgeGhostWindow *>(view);

    if (!window) {
        return;
    }

    xcb_connection_t *c = QX11Info::connection();

    const QByteArray effectName = QByteArrayLiteral("_KDE_NET_WM_SCREEN_EDGE_SHOW");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());

    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));

    if (!atom) {
        return;
    }

    if (!active) {
        xcb_delete_property(c, window->winId(), atom->atom);
        window->hideWithMask();
        return;
    }

    window->showWithMask();

    uint32_t value = 0;

    switch (window->location()) {
    case Plasma::Types::TopEdge:
        value = 0;
        break;

    case Plasma::Types::RightEdge:
        value = 1;
        break;

    case Plasma::Types::BottomEdge:
        value = 2;
        break;

    case Plasma::Types::LeftEdge:
        value = 3;
        break;

    case Plasma::Types::Floating:
    default:
        value = 4;
        break;
    }

    int hideType = 0;

    value |= hideType << 8;

    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->winId(), atom->atom, XCB_ATOM_CARDINAL, 32, 1, &value);
}

QRect XWindowInterface::visibleGeometry(const WindowId &wid, const QRect &frameGeometry) const
{
    NETWinInfo ni(QX11Info::connection(), wid.toUInt(), QX11Info::appRootWindow(), 0, NET::WM2GTKFrameExtents);
    NETStrut struts = ni.gtkFrameExtents();
    QMargins margins(struts.left, struts.top, struts.right, struts.bottom);
    QRect visibleGeometry = frameGeometry;

    if (!margins.isNull()) {
        visibleGeometry -= margins;
    }

    return visibleGeometry;
}



void XWindowInterface::setFrameExtents(QWindow *view, const QMargins &margins)
{
    if (!view) {
        return;
    }

    NETWinInfo ni(QX11Info::connection(), view->winId(), QX11Info::appRootWindow(), 0, NET::WM2GTKFrameExtents);

    if (margins.isNull()) {
        //! delete property
        xcb_connection_t *c = QX11Info::connection();
        const QByteArray atomName = QByteArrayLiteral("_GTK_FRAME_EXTENTS");
        xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, atomName.length(), atomName.constData());
        QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));

        if (!atom) {
            return;
        }

        // qDebug() << "   deleting gtk frame extents atom..";

        xcb_delete_property(c, view->winId(), atom->atom);
    } else {
        NETStrut struts;
        struts.left = margins.left();
        struts.top = margins.top();
        struts.right = margins.right();
        struts.bottom = margins.bottom();

        ni.setGtkFrameExtents(struts);
    }

  /*NETWinInfo ni2(QX11Info::connection(), view->winId(), QX11Info::appRootWindow(), 0, NET::WM2GTKFrameExtents);
    NETStrut applied = ni2.gtkFrameExtents();
    QMargins amargins(applied.left, applied.top, applied.right, applied.bottom);
    qDebug() << "     window gtk frame extents applied :: " << amargins;*/

}

void XWindowInterface::checkShapeExtension()
{
    if (!m_shapeExtensionChecked) {
        xcb_connection_t *c = QX11Info::connection();
        xcb_prefetch_extension_data(c, &xcb_shape_id);
        const xcb_query_extension_reply_t *extension = xcb_get_extension_data(c, &xcb_shape_id);
        if (extension->present) {
            // query version
            auto cookie = xcb_shape_query_version(c);
            QScopedPointer<xcb_shape_query_version_reply_t, QScopedPointerPodDeleter> version(xcb_shape_query_version_reply(c, cookie, nullptr));
            if (!version.isNull()) {
                m_shapeAvailable = (version->major_version * 0x10 + version->minor_version) >= 0x11;
            }
        }
        m_shapeExtensionChecked = true;
    }
}

void XWindowInterface::setInputMask(QWindow *window, const QRect &rect)
{
    if (!window || !window->isVisible()) {
        return;
    }

    xcb_connection_t *c = QX11Info::connection();

    if (!m_shapeExtensionChecked) {
        checkShapeExtension();
    }

    if (!m_shapeAvailable) {
        return;
    }

    if (!rect.isEmpty()) {
        xcb_rectangle_t xcbrect;
        xcbrect.x = qMax(SHRT_MIN, rect.x());
        xcbrect.y = qMax(SHRT_MIN, rect.y());
        xcbrect.width = qMin((int)USHRT_MAX, rect.width());
        xcbrect.height = qMin((int)USHRT_MAX, rect.height());

        // set input shape, so that it doesn't accept any input events
        xcb_shape_rectangles(c, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT,
                             XCB_CLIP_ORDERING_UNSORTED, window->winId(), 0, 0, 1, &xcbrect);
    } else {
        // delete the shape
        xcb_shape_mask(c, XCB_SHAPE_SO_INTERSECT, XCB_SHAPE_SK_INPUT,
                       window->winId(), 0, 0, XCB_PIXMAP_NONE);
    }
}

WindowInfoWrap XWindowInterface::requestInfoActive()
{
    return requestInfo(KWindowSystem::activeWindow());
}

WindowInfoWrap XWindowInterface::requestInfo(WindowId wid)
{
    const KWindowInfo winfo{wid.value<WId>(), NET::WMFrameExtents
                | NET::WMWindowType
                | NET::WMGeometry
                | NET::WMDesktop
                | NET::WMState
                | NET::WMName
                | NET::WMVisibleName,
                NET::WM2WindowClass
                | NET::WM2Activities
                | NET::WM2AllowedActions
                | NET::WM2TransientFor};

    WindowInfoWrap winfoWrap;

    const auto winClass = QString(winfo.windowClassName());

    //!used to track Plasma DesktopView windows because during startup can not be identified properly
    bool plasmaBlockedWindow = (winClass == QLatin1String("plasmashell") && !isAcceptableWindow(wid));

    if (!winfo.valid() || plasmaBlockedWindow) {
        winfoWrap.setIsValid(false);
    } else if (isValidWindow(wid)) {
        winfoWrap.setIsValid(true);
        winfoWrap.setWid(wid);
        winfoWrap.setParentId(winfo.transientFor());
        winfoWrap.setIsActive(KWindowSystem::activeWindow() == wid.value<WId>());
        winfoWrap.setIsMinimized(winfo.hasState(NET::Hidden));
        winfoWrap.setIsMaxVert(winfo.hasState(NET::MaxVert));
        winfoWrap.setIsMaxHoriz(winfo.hasState(NET::MaxHoriz));
        winfoWrap.setIsFullscreen(winfo.hasState(NET::FullScreen));
        winfoWrap.setIsShaded(winfo.hasState(NET::Shaded));
        winfoWrap.setIsOnAllDesktops(winfo.onAllDesktops());
        winfoWrap.setIsOnAllActivities(winfo.activities().empty());
        winfoWrap.setGeometry(visibleGeometry(wid, winfo.frameGeometry()));
        winfoWrap.setIsKeepAbove(winfo.hasState(NET::KeepAbove));
        winfoWrap.setIsKeepBelow(winfo.hasState(NET::KeepBelow));
        winfoWrap.setHasSkipPager(winfo.hasState(NET::SkipPager));
        winfoWrap.setHasSkipSwitcher(winfo.hasState(NET::SkipSwitcher));
        winfoWrap.setHasSkipTaskbar(winfo.hasState(NET::SkipTaskbar));

        //! BEGIN:Window Abilities
        winfoWrap.setIsClosable(winfo.actionSupported(NET::ActionClose));
        winfoWrap.setIsFullScreenable(winfo.actionSupported(NET::ActionFullScreen));
        winfoWrap.setIsMaximizable(winfo.actionSupported(NET::ActionMax));
        winfoWrap.setIsMinimizable(winfo.actionSupported(NET::ActionMinimize));
        winfoWrap.setIsMovable(winfo.actionSupported(NET::ActionMove));
        winfoWrap.setIsResizable(winfo.actionSupported(NET::ActionResize));
        winfoWrap.setIsShadeable(winfo.actionSupported(NET::ActionShade));
        winfoWrap.setIsVirtualDesktopsChangeable(winfo.actionSupported(NET::ActionChangeDesktop));
        //! END:Window Abilities

        winfoWrap.setDisplay(winfo.visibleName());
        winfoWrap.setDesktops({QString(winfo.desktop())});
        winfoWrap.setActivities(winfo.activities());
    }

    if (plasmaBlockedWindow) {
        windowRemoved(wid);
    }

    return winfoWrap;
}

AppData XWindowInterface::appDataFor(WindowId wid)
{
    return appDataFromUrl(windowUrl(wid));
}

QUrl XWindowInterface::windowUrl(WindowId wid)
{
    const KWindowInfo info(wid.value<WId>(), 0, NET::WM2WindowClass | NET::WM2DesktopFileName);

    QString desktopFile = QString::fromUtf8(info.desktopFileName());

    if (!desktopFile.isEmpty()) {
        KService::Ptr service = KService::serviceByStorageId(desktopFile);

        if (service) {
            const QString &menuId = service->menuId();

            // applications: URLs are used to refer to applications by their KService::menuId
            // (i.e. .desktop file name) rather than the absolute path to a .desktop file.
            if (!menuId.isEmpty()) {
                return QUrl(QStringLiteral("applications:") + menuId);
            }

            return QUrl::fromLocalFile(service->entryPath());
        }

        if (!desktopFile.endsWith(QLatin1String(".desktop"))) {
            desktopFile.append(QLatin1String(".desktop"));
        }

        if (KDesktopFile::isDesktopFile(desktopFile) && QFile::exists(desktopFile)) {
            return QUrl::fromLocalFile(desktopFile);
        }
    }

    return windowUrlFromMetadata(info.windowClassClass(),
                                 NETWinInfo(QX11Info::connection(), wid.value<WId>(), QX11Info::appRootWindow(), NET::WMPid, NET::Properties2()).pid(),
                                 rulesConfig, info.windowClassName());
}

bool XWindowInterface::windowCanBeDragged(WindowId wid)
{
    WindowInfoWrap winfo = requestInfo(wid);
    return (winfo.isValid()
            && !winfo.isMinimized()
            && winfo.isMovable()
            && inCurrentDesktopActivity(winfo));
}

bool XWindowInterface::windowCanBeMaximized(WindowId wid)
{
    WindowInfoWrap winfo = requestInfo(wid);
    return (winfo.isValid()
            && !winfo.isMinimized()
            && winfo.isMaximizable()
            && inCurrentDesktopActivity(winfo));
}

void XWindowInterface::requestActivate(WindowId wid)
{
    KWindowSystem::activateWindow(wid.toInt());
}

QIcon XWindowInterface::iconFor(WindowId wid)
{
    QIcon icon;

    icon.addPixmap(KWindowSystem::icon(wid.value<WId>(), KIconLoader::SizeSmall, KIconLoader::SizeSmall, false));
    icon.addPixmap(KWindowSystem::icon(wid.value<WId>(), KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium, false));
    icon.addPixmap(KWindowSystem::icon(wid.value<WId>(), KIconLoader::SizeMedium, KIconLoader::SizeMedium, false));
    icon.addPixmap(KWindowSystem::icon(wid.value<WId>(), KIconLoader::SizeLarge, KIconLoader::SizeLarge, false));

    return icon;
}

WindowId XWindowInterface::winIdFor(QString appId, QRect geometry)
{
    return activeWindow();
}

WindowId XWindowInterface::winIdFor(QString appId, QString title)
{
    return activeWindow();
}

void XWindowInterface::requestClose(WindowId wid)
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid()) {
        return;
    }

    NETRootInfo ri(QX11Info::connection(), NET::CloseWindow);
    ri.closeWindowRequest(wInfo.wid().toUInt());
}

void XWindowInterface::requestMoveWindow(WindowId wid, QPoint from)
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || !inCurrentDesktopActivity(wInfo)) {
        return;
    }

    int borderX = wInfo.geometry().width() > 120 ? 60 : 10;
    int borderY{10};

    //! find min/max values for x,y based on active window geometry
    int minX = wInfo.geometry().x() + borderX;
    int maxX = wInfo.geometry().x() + wInfo.geometry().width() - borderX;
    int minY = wInfo.geometry().y() + borderY;
    int maxY = wInfo.geometry().y() + wInfo.geometry().height() - borderY;

    //! set the point from which this window will be moved,
    //! make sure that it is in window boundaries
    int validX = qBound(minX, from.x(), maxX);
    int validY = qBound(minY, from.y(), maxY);

    NETRootInfo ri(QX11Info::connection(), NET::WMMoveResize);
    ri.moveResizeRequest(wInfo.wid().toUInt(), validX, validY, NET::Move);
}

void XWindowInterface::requestToggleIsOnAllDesktops(WindowId wid)
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid()) {
        return;
    }

    if (KWindowSystem::numberOfDesktops() <= 1) {
        return;
    }

    if (wInfo.isOnAllDesktops()) {
        KWindowSystem::setOnDesktop(wid.toUInt(), KWindowSystem::currentDesktop());
        KWindowSystem::forceActiveWindow(wid.toUInt());
    } else {
        KWindowSystem::setOnAllDesktops(wid.toUInt(), true);
    }
}

void XWindowInterface::requestToggleKeepAbove(WindowId wid)
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid()) {
        return;
    }

    NETWinInfo ni(QX11Info::connection(), wid.toUInt(), QX11Info::appRootWindow(), NET::WMState, NET::Properties2());

    if (wInfo.isKeepAbove()) {
        ni.setState(NET::States(), NET::KeepAbove);
    } else {
        ni.setState(NET::KeepAbove, NET::KeepAbove);
    }
}

void XWindowInterface::setKeepAbove(WindowId wid, bool active)
{
    if (wid.toUInt() <= 0) {
        return;
    }

    if (active) {
        KWindowSystem::setState(wid.toUInt(), NET::KeepAbove);
        KWindowSystem::clearState(wid.toUInt(), NET::KeepBelow);
    } else {
        KWindowSystem::clearState(wid.toUInt(), NET::KeepAbove);
    }
}

void XWindowInterface::setKeepBelow(WindowId wid, bool active)
{
    if (wid.toUInt() <= 0) {
        return;
    }

    if (active) {
        KWindowSystem::setState(wid.toUInt(), NET::KeepBelow);
        KWindowSystem::clearState(wid.toUInt(), NET::KeepAbove);
    } else {
        KWindowSystem::clearState(wid.toUInt(), NET::KeepBelow);
    }
}


void XWindowInterface::requestToggleMinimized(WindowId wid)
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || !inCurrentDesktopActivity(wInfo)) {
        return;
    }

    if (wInfo.isMinimized()) {
        bool onCurrent = wInfo.isOnDesktop(m_currentDesktop);

        KWindowSystem::unminimizeWindow(wid.toUInt());

        if (onCurrent) {
            KWindowSystem::forceActiveWindow(wid.toUInt());
        }
    } else {
        KWindowSystem::minimizeWindow(wid.toUInt());
    }
}

void XWindowInterface::requestToggleMaximized(WindowId wid)
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!windowCanBeMaximized(wid) || !inCurrentDesktopActivity(wInfo)) {
        return;
    }

    bool restore = wInfo.isMaxHoriz() && wInfo.isMaxVert();

    if (wInfo.isMinimized()) {
        KWindowSystem::unminimizeWindow(wid.toUInt());
    }

    NETWinInfo ni(QX11Info::connection(), wid.toInt(), QX11Info::appRootWindow(), NET::WMState, NET::Properties2());

    if (restore) {
        ni.setState(NET::States(), NET::Max);
    } else {
        ni.setState(NET::Max, NET::Max);
    }
}

bool XWindowInterface::isValidWindow(WindowId wid)
{
    if (windowsTracker()->isValidFor(wid)) {
        return true;
    }

    return isAcceptableWindow(wid);
}

bool XWindowInterface::isAcceptableWindow(WindowId wid)
{
    const KWindowInfo info(wid.toUInt(), NET::WMGeometry | NET::WMState, NET::WM2WindowClass);

    const auto winClass = QString(info.windowClassName());

    //! ignored windows do not trackd
    if (hasBlockedTracking(wid)) {
        return false;
    }

    //! whitelisted/approved windows
    if (isWhitelistedWindow(wid)) {
        return true;
    }

    //! Window Checks
    bool hasSkipTaskbar = info.hasState(NET::SkipTaskbar);
    bool hasSkipPager = info.hasState(NET::SkipPager);
    bool isSkipped = hasSkipTaskbar && hasSkipPager;

    if (isSkipped
            && ((winClass == QLatin1String("yakuake")
                 || (winClass == QLatin1String("krunner"))) )) {
        registerWhitelistedWindow(wid);
    } else if (winClass == QLatin1String("plasmashell")) {
        if (isSkipped && isSidepanel(info.geometry())) {
            registerWhitelistedWindow(wid);
            return true;
        } else if (isPlasmaPanel(info.geometry()) || isFullScreenWindow(info.geometry())) {
            registerPlasmaIgnoredWindow(wid);
            return false;
        }
    } else if ((winClass == QLatin1String("latte-dock"))
               || (winClass == QLatin1String("ksmserver"))) {
        if (isFullScreenWindow(info.geometry())) {
            registerIgnoredWindow(wid);
            return false;
        }
    }

    return !isSkipped;
}

void XWindowInterface::windowAddedProxy(WId wid)
{
    if (!isAcceptableWindow(wid)) {
        return;
    }

    emit windowAdded(wid);
    considerWindowChanged(wid);
}

void XWindowInterface::windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2)
{
    if (!isValidWindow(wid)) {
        return;
    }

    //! accept only NET::Properties events,
    //! ignore when the user presses a key, or a window is sending X events etc.
    //! without needing to (e.g. Firefox, https://bugzilla.mozilla.org/show_bug.cgi?id=1389953)
    //! NET::WM2UserTime, NET::WM2IconPixmap etc....
    if (prop1 == 0 && !(prop2 & (NET::WM2Activities | NET::WM2TransientFor))) {
        return;
    }

    //! accept only the following NET:Properties changed signals
    //! NET::WMState, NET::WMGeometry, NET::ActiveWindow
    if ( !(prop1 & NET::WMState)
         && !(prop1 & NET::WMGeometry)
         && !(prop1 & NET::ActiveWindow)
         && !(prop1 & (NET::WMName | NET::WMVisibleName)
              && !(prop2 & NET::WM2TransientFor)
              && !(prop2 & NET::WM2Activities)) ) {
        return;
    }

    considerWindowChanged(wid);
}

}
}
