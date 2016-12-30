#ifndef ABSTRACTWINDOWINTERFACE_H
#define ABSTRACTWINDOWINTERFACE_H

#include "windowinfowrap.h"
#include "../liblattedock/dock.h"

#include <unordered_map>
#include <memory>
#include <list>

#include <QObject>
#include <QRect>
#include <QQuickWindow>

#include <Plasma>

namespace Latte {

class XWindowInterface;

class AbstractWindowInterface : public QObject {
    Q_OBJECT
    
public:
    explicit AbstractWindowInterface(QQuickWindow *const view, QObject *parent = nullptr);
    virtual ~AbstractWindowInterface();
    
    virtual void setDockDefaultFlags() = 0;
    
    virtual WId activeWindow() const = 0;
    virtual WindowInfoWrap requestInfo(WId wid) = 0;
    virtual WindowInfoWrap requestInfoActive() = 0;
    virtual const std::list<WId> &windows() = 0;
    
    virtual void setDockStruts(const QRect &dockRect, Plasma::Types::Location location) = 0;
    virtual void removeDockStruts() = 0;
    
    static AbstractWindowInterface *getInstance(QQuickWindow *const view, QObject *parent = nullptr);
    
signals:
    void activeWindowChanged(WId wid);
    void windowChanged(WId winfo);
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void currentDesktopChanged(int desktop);
    
protected:
    QQuickWindow *const m_view;
    std::list<WId> m_windows;
};

}

#endif // ABSTRACTWINDOWINTERFACE_H
