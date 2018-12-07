#ifndef VISIBILITYMANAGERPRIVATE_H
#define VISIBILITYMANAGERPRIVATE_H

// local
#include "../schemecolors.h"
#include "../wm/abstractwindowinterface.h"
#include "../wm/windowinfowrap.h"
#include "../../liblatte2/types.h"

// C++
#include <array>
#include <memory>

// Qt
#include <QObject>
#include <QTimer>
#include <QEvent>
#include <QVariant>
#include <QMap>

// Plasma
#include <plasmaquick/containmentview.h>

namespace Latte {

class Corona;
class View;
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

    void setMode(Types::Visibility mode);
    void setRaiseOnDesktop(bool enable);
    void setRaiseOnActivity(bool enable);

    void setContainsMouse(bool contains);
    void setIsHidden(bool isHidden);
    void setBlockHiding(bool blockHiding);
    void setTimerShow(int msec);
    void setTimerHide(int msec);

    void raiseView(bool raise);
    void raiseViewTemporarily();
    void updateHiddenState();

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! this is a garbage collector to collect such windows in order to not break the windows array validity.
    void cleanupFaultyWindows();

    //! Dynamic Background Feature
    void setEnabledDynamicBackground(bool active);
    void setExistsWindowMaximized(bool windowMaximized);
    void setExistsWindowSnapped(bool windowSnapped);
    void setTouchingWindowScheme(SchemeColors *scheme);
    void updateAvailableScreenGeometry();
    void updateDynamicBackgroundWindowFlags();

    //! KWin Edges Support functions
    void createEdgeGhostWindow();
    void deleteEdgeGhostWindow();
    void setEnableKWinEdges(bool enable);
    void updateKWinEdgesSupport();

    void setViewGeometry(const QRect &rect);
    void setWindowOnActivities(QWindow &window, const QStringList &activities);
    void applyActivitiesToHiddenWindows(const QStringList &activities);

    void windowAdded(WindowId id);
    void dodgeActive(WindowId id);
    void dodgeMaximized(WindowId id);
    void dodgeWindows(WindowId id);
    void checkAllWindows();

    bool intersects(const WindowInfoWrap &winfo);
    bool isMaximizedInCurrentScreen(const WindowInfoWrap &winfo);
    bool isTouchingPanelEdge(const WindowInfoWrap &winfo);

    void updateStrutsBasedOnLayoutsAndActivities();

    void requestToggleMaximizeForActiveWindow();
    void requestMoveActiveWindow(int localX, int localY);
    bool activeWindowCanBeDragged();

    void saveConfig();
    void restoreConfig();

    void viewEventManager(QEvent *ev);

    VisibilityManager *q;
    PlasmaQuick::ContainmentView *view;
    AbstractWindowInterface *wm;
    Types::Visibility mode{Types::None};
    std::array<QMetaObject::Connection, 5> connections;
    QMap<WindowId, WindowInfoWrap> windows;

    QTimer timerShow;
    QTimer timerHide;
    QTimer timerCheckWindows;
    QTimer timerStartUp;
    QRect m_viewGeometry;
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
    SchemeColors *touchingScheme{nullptr};


    //! KWin Edges
    bool enableKWinEdgesFromUser{true};
    std::array<QMetaObject::Connection, 1> connectionsKWinEdges;
    ScreenEdgeGhostWindow *edgeGhostWindow{nullptr};

    Latte::Corona *m_corona{nullptr};
    Latte::View *m_latteView{nullptr};
};

}

#endif // VISIBILITYMANAGERPRIVATE_H
