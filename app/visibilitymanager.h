#ifndef VISIBILITYMANAGER_H
#define VISIBILITYMANAGER_H

#include "plasmaquick/containmentview.h"
#include "../liblattedock/dock.h"

#include <QObject>
#include <QTimer>

#include <Plasma/Containment>

class VisibilityManager : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(Latte::Dock::Visibility mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool isHidden READ isHidden WRITE isHidden NOTIFY isHiddenChanged)
    Q_PROPERTY(bool containsMouse READ containsMouse NOTIFY containsMouseChanged)
    Q_PROPERTY(int timerShow READ timerShow WRITE setTimerShow NOTIFY timerShowChanged)
    Q_PROPERTY(int timerHide READ timerHide WRITE setTimerHide NOTIFY timerHideChanged)
    
public:
    explicit VisibilityManager(PlasmaQuick::ContainmentView *view);
    virtual ~VisibilityManager();
    
    Latte::Dock::Visibility mode() const;
    void setMode(Latte::Dock::Visibility mode);
    
    bool isHidden() const;
    void setHidden(bool isHidden);
    
    bool containsMouse() const;
    
    int timerShow() const;
    void setTimerShow(int msec);
    
    int timerHide() const;
    void setTimerHide(int msec);
    
    /**
     * @brief updateDockGeometry, the window geometry in absolute coordinates.
     */
    void updateDockGeometry(QRect &geometry);
    
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
#endif // VISIBILITYMANAGER_H
