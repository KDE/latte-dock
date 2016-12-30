#ifndef VISIBILITYMANAGER_H
#define VISIBILITYMANAGER_H

#include "plasmaquick/containmentview.h"
#include "../liblattedock/dock.h"

#include <QObject>
#include <QTimer>

#include <Plasma/Containment>

namespace Latte {

class VisibilityManagerPrivate;

class VisibilityManager : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(Latte::Dock::Visibility mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool isHidden READ isHidden WRITE setIsHidden NOTIFY isHiddenChanged)
    Q_PROPERTY(bool containsMouse READ containsMouse NOTIFY containsMouseChanged)
    Q_PROPERTY(int timerShow READ timerShow WRITE setTimerShow NOTIFY timerShowChanged)
    Q_PROPERTY(int timerHide READ timerHide WRITE setTimerHide NOTIFY timerHideChanged)
    
public:
    explicit VisibilityManager(PlasmaQuick::ContainmentView *view);
    virtual ~VisibilityManager();
    
    Latte::Dock::Visibility mode() const;
    void setMode(Latte::Dock::Visibility mode);
    
    bool isHidden() const;
    void setIsHidden(bool isHidden);
    
    bool containsMouse() const;
    
    int timerShow() const;
    void setTimerShow(int msec);
    
    int timerHide() const;
    void setTimerHide(int msec);
    
    /**
     * @brief updateDockGeometry, the window geometry in absolute coordinates.
     */
    void updateDockGeometry(const QRect &geometry);
    
signals:
    void mustBeShown();
    void mustBeHide();
    
    void modeChanged();
    void isHiddenChanged();
    void containsMouseChanged();
    void timerShowChanged();
    void timerHideChanged();
    
private:
    VisibilityManagerPrivate *const d;
};

}
#endif // VISIBILITYMANAGER_H
