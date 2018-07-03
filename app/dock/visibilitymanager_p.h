#ifndef VISIBILITYMANAGERPRIVATE_H
#define VISIBILITYMANAGERPRIVATE_H

#include "../windowinfowrap.h"
#include "../abstractwindowinterface.h"
#include "../../liblattedock/dock.h"

#include <array>
#include <memory>

#include <QObject>
#include <QTimer>
#include <QEvent>
#include <QVariant>
#include <QMap>

#include <plasmaquick/containmentview.h>

namespace Latte {

class DockCorona;
class DockView;
class VisibilityManager;
class ScreenEdgeGhostWindow;

/*!
 * \brief The Latte::VisibilityManagerPrivate is a class d-pointer
 */
class VisibilityManagerPrivate : public QObject
{
    Q_GADGET

public:
    VisibilityManagerPrivate(PlasmaQuick::ContainmentView *view, VisibilityManager *q);
    ~VisibilityManagerPrivate();

    void setMode(Dock::Visibility mode);
    void setRaiseOnDesktop(bool enable);
    void setRaiseOnActivity(bool enable);

    void setIsHidden(bool isHidden);
    void setBlockHiding(bool blockHiding);
    void setTimerShow(int msec);
    void setTimerHide(int msec);

    void raiseDock(bool raise);
    void raiseDockTemporarily();
    void updateHiddenState();

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! this is a garbage collector to collect such windows in order to not break the windows array validity.
    void cleanupFaultyWindows();

    //! Dynamic Background Feature
    void setEnabledDynamicBackground(bool active);
    void setExistsWindowMaximized(bool windowMaximized);
    void setExistsWindowSnapped(bool windowSnapped);
    void updateAvailableScreenGeometry();
    void updateDynamicBackgroundWindowFlags();

    //! KWin Edges Support functions
    void createEdgeGhostWindow();
    void deleteEdgeGhostWindow();
    void setEnableKWinEdges(bool enable);
    void updateKWinEdgesSupport();

    void setDockGeometry(const QRect &rect);
    void setWindowOnActivities(QWindow &window, const QStringList &activities);
    void applyActivitiesToHiddenWindows(const QStringList &activities);

    void windowAdded(WindowId id);
    void dodgeActive(WindowId id);
    void dodgeMaximized(WindowId id);
    void dodgeWindows(WindowId id);
    void checkAllWindows();

    bool intersects(const WindowInfoWrap &winfo);

    void updateStrutsBasedOnLayoutsAndActivities();

    void saveConfig();
    void restoreConfig();

    void viewEventManager(QEvent *ev);

    VisibilityManager *q;
    PlasmaQuick::ContainmentView *view;
    AbstractWindowInterface *wm;
    Dock::Visibility mode{Dock::None};
    std::array<QMetaObject::Connection, 5> connections;
    QMap<WindowId, WindowInfoWrap> windows;

    QTimer timerShow;
    QTimer timerHide;
    QTimer timerCheckWindows;
    QTimer timerStartUp;
    QRect dockGeometry;
    bool isHidden{false};
    bool dragEnter{false};
    bool blockHiding{false};
    bool containsMouse{false};
    bool raiseTemporarily{false};
    bool raiseOnDesktopChange{false};
    bool raiseOnActivityChange{false};
    bool hideNow{false};

    //! Dynamic Background flags and needed information
    bool enabledDynamicBackgroundFlag{false};
    bool windowIsSnappedFlag{false};
    bool windowIsMaximizedFlag{false};
    QRect availableScreenGeometry;
    QList<QRect> snappedWindowsGeometries;
    std::array<QMetaObject::Connection, 7> connectionsDynBackground;
    WindowId lastActiveWindowWid;

    //! KWin Edges
    bool enableKWinEdgesFromUser{true};
    std::array<QMetaObject::Connection, 1> connectionsKWinEdges;
    ScreenEdgeGhostWindow *edgeGhostWindow{nullptr};

    DockCorona *dockCorona{nullptr};
    DockView *dockView{nullptr};
};

}

#endif // VISIBILITYMANAGERPRIVATE_H
