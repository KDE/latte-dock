#ifndef XWINDOWINTERFACE_H
#define XWINDOWINTERFACE_H

#include <QObject>

#include <KWindowInfo>

#include "abstractinterface.h"

namespace NowDock {

class XWindowInterface : public AbstractInterface {
    Q_OBJECT
    
public:
    explicit XWindowInterface(QQuickWindow *parent);
    ~XWindowInterface();
    
    bool activeIsDialog() const;
    bool activeIsMaximized() const;
    bool dockIntersectsActiveWindow() const;
    bool desktopIsActive() const;
    bool dockIsCovered(bool totally = false) const;
    bool dockIsCovering() const;
    bool dockIsOnTop() const;
    bool dockInNormalState() const;
    bool dockIsBelow() const;
    
    void setDockDefaultFlags(bool dock = false);
    void setDockToAllDesktops();
    void setDockToAlwaysVisible();
    void showDockAsNormal();
    void showDockOnBottom();
    void showDockOnTop();
    
private Q_SLOTS:
    void activeWindowChanged(WId win);
    void dockNumberChanged(unsigned int no);
    void windowChanged(WId id, NET::Properties properties, NET::Properties2 properties2);
    void windowRemoved(WId id);
    
private:
    WId m_activeWindow;
    WId m_demandsAttention;
    
    bool isDesktop(WId id) const;
    bool isDialog(WId id) const;
    bool isMaximized(WId id) const;
    bool isNormal(WId id) const;
    bool isOnBottom(WId id) const;
    bool isOnTop(WId id) const;
};

}

#endif




