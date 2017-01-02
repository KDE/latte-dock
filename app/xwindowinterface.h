#ifndef XWINDOWINTERFACE_H
#define XWINDOWINTERFACE_H

#include "abstractwindowinterface.h"

#include <QObject>

#include <KWindowInfo>
#include <Plasma>

namespace Latte {

class XWindowInterface : public AbstractWindowInterface {
    Q_OBJECT
    
public:
    XWindowInterface(QQuickWindow *const view, QObject *parent);
    virtual ~XWindowInterface();
    
    void setDockDefaultFlags() override;
    
    WId activeWindow() const override;
    WindowInfoWrap requestInfo(WId wid) const override;
    WindowInfoWrap requestInfoActive() const override;
    bool isOnCurrentDesktop(WId wid) const override;
    const std::list<WId> &windows() const override;
    
    void setDockStruts(const QRect &dockRect, Plasma::Types::Location location) const override;
    void removeDockStruts() const override;
    
    
private:
    bool isValidWindow(const KWindowInfo &winfo) const;
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);
    
    WId m_desktopId;
};

}

#endif // XWINDOWINTERFACE_H




