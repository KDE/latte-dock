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
    ~XWindowInterface() override;
    
    void setDockDefaultFlags() override;
    
    WId activeWindow() const override;
    WindowInfoWrap requestInfo(WId wid) override;
    WindowInfoWrap requestInfoActive() override;
    const std::list<WId> &windows() override;
    
    void setDockStruts(const QRect &dockRect, Plasma::Types::Location location) override;
    void removeDockStruts() override;
    
private:
    bool isValidWindow(const KWindowInfo &winfo);
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);
};

}

#endif // XWINDOWINTERFACE_H




