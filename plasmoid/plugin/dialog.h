/*
 *  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef LATTEDIALOG_H
#define LATTEDIALOG_H

// Qt
#include <QEvent>
#include <QObject>

// Plasma
#include <PlasmaQuick/Dialog>

namespace Latte {
namespace Quick {

class Dialog : public PlasmaQuick::Dialog {
    Q_OBJECT
    Q_PROPERTY (bool containsMouse READ containsMouse NOTIFY containsMouseChanged)

public:
    explicit Dialog(QQuickItem *parent = nullptr);

    bool containsMouse() const;

signals:
    void containsMouseChanged();

protected:
    bool event(QEvent *e) override;

private slots:
    void setContainsMouse(bool contains);

private:
    bool m_containsMouse{false};


};

}
}

#endif
