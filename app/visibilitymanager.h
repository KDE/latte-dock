#ifndef VISIBILITYMANAGER_H
#define VISIBILITYMANAGER_H

#include "abstractinterface.h"
#include "plasmaquick/containmentview.h"
#include "../liblattedock/dock.h"

#include <QObject>
#include <QTimer>

#include <Plasma/Containment>

class VisibilityManager : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(Latte::Dock::Visibility mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(int timerShow READ timerShow WRITE setTimerShow NOTIFY timerShowChanged)
    Q_PROPERTY(int timerHide READ timerHide WRITE setTimerHide NOTIFY timerHideChanged)
    Q_PROPERTY(Latte::Dock::VisibilityState state READ state WRITE setState NOTIFY stateChanged)
    
public:
    explicit VisibilityManager(PlasmaQuick::ContainmentView *view);
    virtual ~VisibilityManager();
    
    Latte::Dock::Visibility mode() const;
    void setMode(Latte::Dock::Visibility mode);
    
    int timerShow() const;
    void setTimerShow(int msec);
    
    int timerHide() const;
    void setTimerHide(int msec);
    
    /**
     * @brief show, change state to Dock::Visible when show timer is triggered.
     */
    Q_INVOKABLE void show();
    
    /**
     * @brief showImmediately, same that show, but without timer
     */
    Q_INVOKABLE void showImmediately();
    
    /**
     * @brief restore, change to last state, respecting the timers.
     */ 
    Q_INVOKABLE void restore();
    
    /**
     * @brief showTemporarily, same that show but restores last state automatically
     * @param msec, timeout before restoring
     */
    Q_INVOKABLE void showTemporarily(int msec);
    
    /**
     * @brief updateDockGeometry, the geometry should be inside screen, not into window.
     */
    void updateDockGeometry(QRect &geometry);
        
signals:
    /**
     * @brief mouseEntered, emitted when mouse enters the dock
     */
    void mouseEntered();
    
    /**
     * @brief mouseExited, emitted when mouse leaves the dock
     */
    void mouseExited();
 
    void modeChanged();
    void timerShowChanged();
    void timerHideChanged();
    void stateChanged();
};
#endif // VISIBILITYMANAGER_H
