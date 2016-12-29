#ifndef ABSTRACTWINDOWINTERFACE_H
#define ABSTRACTWINDOWINTERFACE_H

#include "windowinfowrap.h"
#include "../liblattedock/dock.h"

#include <unordered_map>
#include <memory>
#include <list>

#include <QObject>
#include <QQuickWindow>

namespace Latte {

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
    
signals:
    void activeWindowChanged(WId wid);
    void windowChanged(const WindowInfoWrap &winfo);
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void currentDesktopChanged(int desktop);
    
protected:
    QQuickWindow *const m_view;
    std::list<WId> m_windows;
};

}

#endif // ABSTRACTWINDOWINTERFACE_H
