#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include "abstractinterface.h"

namespace NowDock {

class WindowSystem : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(bool compositingActive READ compositingActive NOTIFY compositingChanged)
    
public:
    explicit WindowSystem(QObject *parent = Q_NULLPTR);
    ~WindowSystem();
    
    bool compositingActive() const;
    
Q_SIGNALS:
    void compositingChanged();
    
private Q_SLOTS:
    void compositingChanged(bool state);
};

}//NowDock namespace

#endif
