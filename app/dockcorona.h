/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NOWDOCKCORONA_H
#define NOWDOCKCORONA_H

#include "dockview.h"

#include <QObject>

namespace Plasma {
class Corona;
class Containment;
class Types;
}

namespace KActivities {
class Consumer;
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
    
    int docksCount(int screen) const;
    int screenForContainment(const Plasma::Containment *containment) const override;
    
    void addDock(Plasma::Containment *containment);
    
    void closeApplication();
    
public slots:
    void loadDefaultLayout() override;
    void dockContainmentDestroyed(QObject *cont);
    
signals:
    void configurationShown(PlasmaQuick::ConfigView *configView);
    void docksCountChanged();
    
private slots:
    void destroyedChanged(bool destroyed);
    void load();
    
private:
    void qmlRegisterTypes() const;
    int primaryScreenId() const;
    
    QHash<const Plasma::Containment *, DockView *> m_dockViews;
    QHash<const Plasma::Containment *, DockView *> m_waitingDockViews;
    
    KActivities::Consumer *m_activityConsumer;
};

}

#endif
