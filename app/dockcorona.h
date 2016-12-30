#ifndef NOWDOCKCORONA_H
#define NOWDOCKCORONA_H

#include "dockview.h"

#include <QObject>

namespace Plasma {
class Corona;
class Containment;
class Types;
}

namespace Latte {

class DockCorona : public Plasma::Corona {
    Q_OBJECT
    
public:
    DockCorona(QObject *parent = nullptr);
    virtual ~DockCorona();
    
    int numScreens() const override;
    QRect screenGeometry(int id) const override;
    QRegion availableScreenRegion(int id) const override;
    QRect availableScreenRect(int id) const override;
    
    QList<Plasma::Types::Location> freeEdges(int screen) const;
    
    int screenForContainment(const Plasma::Containment *containment) const override;
    
    void addDock(Plasma::Containment *containment);
    
public slots:
    void loadDefaultLayout() override;
    
signals:
    void configurationShown(PlasmaQuick::ConfigView *configView);
    
private:
    void qmlRegisterTypes() const;
    int primaryScreenId() const;
    
    std::vector<DockView *> m_containments;
};

}

#endif
