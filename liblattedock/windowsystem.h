#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <QObject>

namespace Latte {

class WindowSystem : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(bool compositingActive READ compositingActive NOTIFY compositingChanged)
    
public:
    explicit WindowSystem(QObject *parent = nullptr);
    ~WindowSystem();
    
    static WindowSystem &self();
    
    bool compositingActive() const;
    
signals:
    void compositingChanged();
    
private slots:
    void compositingChangedProxy(bool state);
    
private:
    bool m_compositing{false};
};

}//LatteDock namespace

#endif
