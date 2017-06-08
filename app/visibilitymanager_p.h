#ifndef VISIBILITYMANAGERPRIVATE_H
#define VISIBILITYMANAGERPRIVATE_H

#include "../liblattedock/dock.h"
#include "windowinfowrap.h"
#include "abstractwindowinterface.h"

#include <memory>

#include <QObject>
#include <QTimer>
#include <QEvent>
#include <QVariant>
#include <QMap>

#include <plasmaquick/containmentview.h>

namespace Latte {

class VisibilityManager;

/*!
 * \brief The Latte::VisibilityManagerPrivate is a class d-pointer
 */
class VisibilityManagerPrivate : public QObject {
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

    void setDockGeometry(const QRect &rect);

    void windowAdded(WindowId id);
    void dodgeActive(WindowId id);
    void dodgeMaximized(WindowId id);
    void dodgeWindows(WindowId id);
    void checkAllWindows();

    bool intersects(const WindowInfoWrap &winfo);

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
};

}

#endif // VISIBILITYMANAGERPRIVATE_H
