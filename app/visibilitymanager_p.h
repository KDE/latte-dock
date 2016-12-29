#ifndef VISIBILITYMANAGERPRIVATE_H
#define VISIBILITYMANAGERPRIVATE_H

#include "../liblattedock/dock.h"
#include "windowinfowrap.h"

#include <memory>
#include <unordered_map>

#include <QObject>
#include <QTimer>
#include <QEvent>

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
    void setTimerShow(int msec);
    void setTimerHide(int msec);
    
    void raiseDock(bool raise);
    
    void setDockRect(const QRect &rect);
    
    void windowAdded(WId id);
    void dodgeActive(WId id);
    void dodgeWindows(WId id);
    void checkAllWindows();
    
    bool intersects(const WindowInfoWrap &info);
    
    void saveConfig();
    void restoreConfig();
    
    bool event(QEvent *ev) override;
    
    VisibilityManager *q;
    Dock::Visibility mode{Dock::DodgeActive};
    std::array<QMetaObject::Connection, 4> connections;
    std::unordered_map<WId, std::unique_ptr<WindowInfoWrap>> windows;
    QTimer timerShow;
    QTimer timerHide;
    QTimer timerCheckWindows;
    QRect dockRect;
    bool isHidden{false};
    bool containsMouse{false};
};

}

#endif // VISIBILITYMANAGERPRIVATE_H
