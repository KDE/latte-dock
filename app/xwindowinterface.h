#ifndef XWINDOWINTERFACE_H
#define XWINDOWINTERFACE_H

#include <QObject>

#include <KWindowInfo>

#include "abstractwindowinterface.h"

namespace Latte {

class XWindowInterface : public AbstractWindowInterface {
    Q_OBJECT
    
public:
    XWindowInterface(QQuickWindow *const view, QObject *parent);
    virtual ~XWindowInterface();
    
    void setDockDefaultFlags() override;
    
    WId activeWindow() const override;
    WindowInfoWrap requestInfo(WId wid) override;
    WindowInfoWrap requestInfoActive() override;
    const std::list<WId> &windows() override;
    
private:
    bool isValidWindow(const KWindowInfo &winfo);
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);
};

}

#endif // XWINDOWINTERFACE_H




