#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <QObject>

namespace Latte {

class WindowSystem : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(bool compositingActive READ compositingActive NOTIFY compositingChanged)
    
public:
    explicit WindowSystem(QObject *parent = nullptr);
    
    static WindowSystem &self();
    ~WindowSystem();
    
    bool compositingActive() const;
    
signals:
    void compositingChanged();
    
private slots:
    void compositingChangedProxy(bool enabled);
    
private:
    bool m_enabled{false};
};

}//LatteDock namespace

#endif
