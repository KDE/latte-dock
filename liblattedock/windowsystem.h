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
