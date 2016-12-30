/*
 * Copyright 2014  Bhushan Shah <bhush94@gmail.com>
 * Copyright 2014 Marco Martin <notmart@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef NOWDOCKCORONA_H
#define NOWDOCKCORONA_H

#include <QObject>

#include "nowdockview.h"


namespace Plasma {
class Corona;
class Containment;
class Types;
}

namespace Latte {

class NowDockCorona : public Plasma::Corona {
    Q_OBJECT
    
public:
    NowDockCorona(QObject *parent = nullptr);
    ~NowDockCorona() override;
    
    int numScreens() const override;
    QRect screenGeometry(int id) const override;
    QRegion availableScreenRegion(int id) const override;
    QRect availableScreenRect(int id) const override;
    
    QList<Plasma::Types::Location> freeEdges(int screen) const;
    
    int screenForContainment(const Plasma::Containment *containment) const override;
    
    void addDock(Plasma::Containment *containment);
    
public slots:
    void loadDefaultLayout() override;
    
private:
    void qmlRegisterTypes() const;
    
    std::vector<NowDockView *> m_containments;
};

}

#endif
