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

#include "xwindowinterface.h"

// local
#include "tasktools.h"
#include "view/screenedgeghostwindow.h"
#include "view/view.h"
#include "../liblatte2/extras.h"

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

namespace Latte {
namespace WindowSystem {

XWindowInterface::XWindowInterface(QObject *parent)
    : AbstractWindowInterface(parent)
{
    m_currentDesktop = QString(KWindowSystem::self()->currentDesktop());

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &AbstractWindowInterface::activeWindowChanged);
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &AbstractWindowInterface::windowAdded);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &AbstractWindowInterface::windowRemoved);

    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [&](int desktop) {
        m_currentDesktop = QString(desktop);
        emit currentDesktopChanged();
    });

    connect(KWindowSystem::self()
            , static_cast<void (KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>
            (&KWindowSystem::windowChanged)
            , this, &XWindowInterface::windowChangedProxy);


    for(auto wid : KWindowSystem::self()->windows()) {
        emit windowAdded(wid);
        windowChangedProxy(wid,0,0);
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

    if (winId < 0) {
        return;
    }

    NETWinInfo winfo(QX11Info::connection()
                     , static_cast<xcb_window_t>(winId)
                     , static_cast<xcb_window_t>(winId)
                     , 0, 0);

    winfo.setAllowedActions(NET::ActionChangeDesktop);

    if (isPanelWindow) {
        KWindowSystem::setType(winId, NET::Dock);
    }

    KWindowSystem::setState(winId, NET::SkipTaskbar | NET::SkipPager);
    KWindowSystem::setOnAllDesktops(winId, true);
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
        qWarning() << "wrong location:" << qEnumToStr(location);
        return;
    }

    KWindowSystem::setExtendedStrut(view.winId(),
                                    strut.left_width,   strut.left_start,   strut.left_end,
                                    strut.right_width,  strut.right_start,  strut.right_end,
                                    strut.top_width,    strut.top_start,    strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end
                                    );
}

void XWindowInterface::switchToNextVirtualDesktop() const
{
    int desktops = KWindowSystem::numberOfDesktops();

    if (desktops <= 1) {
        return;
    }

    int curPos = KWindowSystem::currentDesktop();
    int nextPos = curPos + 1;

    if (curPos == desktops) {
        nextPos = 1;
    }

    KWindowSystem::setCurrentDesktop(nextPos);
}

void XWindowInterface::switchToPreviousVirtualDesktop() const
{
    int desktops = KWindowSystem::numberOfDesktops();
    if (desktops <= 1) {
        return;
    }

    int curPos = KWindowSystem::currentDesktop();
    int nextPos = curPos - 1;

    if (curPos == 1) {
        nextPos = desktops;
    }

    KWindowSystem::setCurrentDesktop(nextPos);
}

void XWindowInterface::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    KWindowSystem::setOnActivities(window.winId(), activities);
}

void XWindowInterface::removeViewStruts(QWindow &view) const
{
    KWindowSystem::setStrut(view.winId(), 0, 0, 0, 0);
}

WindowId XWindowInterface::activeWindow() const
{
    return KWindowSystem::self()->activeWindow();
}

void XWindowInterface::setKeepAbove(const QDialog &dialog, bool above) const
{
    if (above) {
        KWindowSystem::setState(dialog.winId(), NET::KeepAbove);
    } else {
        KWindowSystem::clearState(dialog.winId(), NET::KeepAbove);
    }
}

void XWindowInterface::skipTaskBar(const QDialog &dialog) const
{
    KWindowSystem::setState(dialog.winId(), NET::SkipTaskbar);
}

void XWindowInterface::slideWindow(QWindow &view, AbstractWindowInterface::Slide location) const
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

void XWindowInterface::enableBlurBehind(QWindow &view) const
{
    KWindowEffects::enableBlurBehind(view.winId());
}

void XWindowInterface::setEdgeStateFor(QWindow *view, bool active) const
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

WindowInfoWrap XWindowInterface::requestInfoActive() const
{
    return requestInfo(KWindowSystem::activeWindow());
}

WindowInfoWrap XWindowInterface::requestInfo(WindowId wid) const
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
                | NET::WM2TransientFor};

    //! update desktop id

    bool isDesktop{false};
    if (winfo.windowClassName() == "plasmashell" && isPlasmaDesktop(winfo.geometry())) {
        isDesktop = true;
        windowsTracker()->setPlasmaDesktop(wid);
    }

    WindowInfoWrap winfoWrap;

    if (!winfo.valid()) {
        winfoWrap.setIsValid(false);
    } else if (isValidWindow(winfo) && !isDesktop) {
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
        winfoWrap.setGeometry(winfo.frameGeometry());
        winfoWrap.setIsKeepAbove(winfo.hasState(NET::KeepAbove));
        winfoWrap.setIsKeepBelow(winfo.hasState(NET::KeepBelow));
        winfoWrap.setHasSkipTaskbar(winfo.hasState(NET::SkipTaskbar));
        winfoWrap.setDisplay(winfo.visibleName());

        winfoWrap.setDesktops({QString(winfo.desktop())});
        winfoWrap.setActivities(winfo.activities());
    } else if (m_desktopId == wid) {
        winfoWrap.setIsValid(true);
        winfoWrap.setIsPlasmaDesktop(true);
        winfoWrap.setWid(wid);
        winfoWrap.setParentId(0);
        winfoWrap.setHasSkipTaskbar(true);
    }

    return winfoWrap;
}

AppData XWindowInterface::appDataFor(WindowId wid) const
{
    return appDataFromUrl(windowUrl(wid));
}

QUrl XWindowInterface::windowUrl(WindowId wid) const
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


bool XWindowInterface::windowCanBeDragged(WindowId wid) const
{
    KWindowInfo info(wid.value<WId>(), 0, NET::WM2AllowedActions);

    if (info.valid()) {
        WindowInfoWrap winfo = requestInfo(wid);
        return (winfo.isValid()
                && info.actionSupported(NET::ActionMove)
                && !winfo.isMinimized()
                && inCurrentDesktopActivity(winfo)
                && !winfo.isPlasmaDesktop());
    }

    return false;
}

bool XWindowInterface::windowCanBeMaximized(WindowId wid) const
{
    KWindowInfo info(wid.value<WId>(), 0, NET::WM2AllowedActions);

    if (info.valid()) {
        WindowInfoWrap winfo = requestInfo(wid);
        return (winfo.isValid()
                && !winfo.isMinimized()
                && info.actionSupported(NET::ActionMax)
                && inCurrentDesktopActivity(winfo)
                && !winfo.isPlasmaDesktop());
    }

    return false;
}

void XWindowInterface::requestActivate(WindowId wid) const
{
    KWindowSystem::activateWindow(wid.toInt());
}

QIcon XWindowInterface::iconFor(WindowId wid) const
{
    QIcon icon;

    icon.addPixmap(KWindowSystem::icon(wid.value<WId>(), KIconLoader::SizeSmall, KIconLoader::SizeSmall, false));
    icon.addPixmap(KWindowSystem::icon(wid.value<WId>(), KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium, false));
    icon.addPixmap(KWindowSystem::icon(wid.value<WId>(), KIconLoader::SizeMedium, KIconLoader::SizeMedium, false));
    icon.addPixmap(KWindowSystem::icon(wid.value<WId>(), KIconLoader::SizeLarge, KIconLoader::SizeLarge, false));

    return icon;
}

WindowId XWindowInterface::winIdFor(QString appId, QRect geometry) const
{
    return activeWindow();
}

void XWindowInterface::requestClose(WindowId wid) const
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || wInfo.isPlasmaDesktop()) {
        return;
    }

    NETRootInfo ri(QX11Info::connection(), NET::CloseWindow);
    ri.closeWindowRequest(wInfo.wid().toUInt());
}

void XWindowInterface::requestMoveWindow(WindowId wid, QPoint from) const
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || wInfo.isPlasmaDesktop() || !inCurrentDesktopActivity(wInfo)) {
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

void XWindowInterface::requestToggleIsOnAllDesktops(WindowId wid) const
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || wInfo.isPlasmaDesktop()) {
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

void XWindowInterface::requestToggleKeepAbove(WindowId wid) const
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || wInfo.isPlasmaDesktop()) {
        return;
    }

    NETWinInfo ni(QX11Info::connection(), wid.toUInt(), QX11Info::appRootWindow(), NET::WMState, NET::Properties2());

    if (wInfo.isKeepAbove()) {
        ni.setState(NET::States(), NET::StaysOnTop);
    } else {
        ni.setState(NET::StaysOnTop, NET::StaysOnTop);
    }
}

void XWindowInterface::setKeepAbove(WindowId wid, bool active) const
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || wInfo.isPlasmaDesktop()) {
        return;
    }

    NETWinInfo ni(QX11Info::connection(), wid.toUInt(), QX11Info::appRootWindow(), NET::WMState, NET::Properties2());

    if ((wInfo.isKeepAbove() && active) || (!wInfo.isKeepAbove() && !active)) {
        return;
    }

    if (wInfo.isKeepAbove()) {
        ni.setState(NET::States(), NET::StaysOnTop);
    } else {
        ni.setState(NET::StaysOnTop, NET::StaysOnTop);
    }
}

void XWindowInterface::setKeepBelow(WindowId wid, bool active) const
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || wInfo.isPlasmaDesktop()) {
        return;
    }

    NETWinInfo ni(QX11Info::connection(), wid.toUInt(), QX11Info::appRootWindow(), NET::WMState, NET::Properties2());

    if ((wInfo.isKeepBelow() && active) || (!wInfo.isKeepBelow() && !active)) {
        return;
    }

    if (wInfo.isKeepBelow()) {
        ni.setState(NET::States(), NET::KeepBelow);
    } else {
        ni.setState(NET::KeepBelow, NET::KeepBelow);
    }
}


void XWindowInterface::requestToggleMinimized(WindowId wid) const
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || wInfo.isPlasmaDesktop() || !inCurrentDesktopActivity(wInfo)) {
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

void XWindowInterface::requestToggleMaximized(WindowId wid) const
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

bool XWindowInterface::isValidWindow(WindowId wid) const
{
    if (windowsTracker()->isValidFor(wid)) {
        return true;
    }

    const KWindowInfo winfo{wid.value<WId>(), NET::WMWindowType};

    return isValidWindow(winfo);
}

bool XWindowInterface::isValidWindow(const KWindowInfo &winfo) const
{
    if (windowsTracker()->isValidFor(winfo.win())) {
        return true;
    }

    constexpr auto types = NET::DockMask | NET::MenuMask | NET::SplashMask | NET::PopupMenuMask | NET::NormalMask | NET::DialogMask;
    NET::WindowType winType = winfo.windowType(types);
    const auto winClass = KWindowInfo(winfo.win(), 0, NET::WM2WindowClass).windowClassName();

    //! ignored windows from tracking
    if (m_ignoredWindows.contains(winfo.win())) {
        return false;
    }

    if (m_desktopId == winfo.win()) {
        return false;
    }

    if (winType == -1) {
        // Trying to get more types for verify if the window have any other type
        winType = winfo.windowType(~types & NET::AllTypesMask);

        if (winType == -1) {
            qWarning() << KWindowInfo(winfo.win(), 0, NET::WM2WindowClass).windowClassName()
                       << "doesn't have any WindowType, assuming as NET::Normal";
            return true;
        }
    }

    bool isMenu = ((winType & NET::Menu) == true);
    bool isDock = ((winType & NET::Dock) == true);
    bool isPopup = ((winType & NET::PopupMenu) == true);
    bool isSplash = ((winType & NET::Splash) == true);

    //! GTK2+ dialogs case e.g. inkscape, gimp2, etc...
    //! are both Popups and Splash types, this is why
    //! we can not black list them here
    return !(isMenu || isDock);
}

void XWindowInterface::windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2)
{
    const KWindowInfo info(wid, NET::WMGeometry, NET::WM2WindowClass);

    const auto winClass = info.windowClassName();

    //! ignored windows do not trackd
    if (m_ignoredWindows.contains(wid)) {
        return;
    }

    if (winClass == "plasmashell") {
        //! update desktop id
        if (isPlasmaDesktop(info.geometry())) {
            m_desktopId = wid;
            windowsTracker()->setPlasmaDesktop(wid);
            considerWindowChanged(wid);
            return;
        } else if (isPlasmaPanel(info.geometry())) {
            registerPlasmaPanel(wid);
            return;
        }
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
         && !(prop1 & NET::WMDesktop)
         && !(prop1 & (NET::WMName | NET::WMVisibleName)
              && !(prop2 & NET::WM2TransientFor)
              && !(prop2 & NET::WM2Activities)) ) {
        return;
    }

    //! ignore windows that do not respect normal windows types
    if (!isValidWindow(wid)) {
        return;
    }

    considerWindowChanged(wid);
}

}
}
