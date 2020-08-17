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

#ifndef LAYOUTSSTORAGE_H
#define LAYOUTSSTORAGE_H

// Qt
#include <QTemporaryDir>

// KDE
#include <KConfigGroup>

// Plasma
#include <Plasma/Containment>

namespace Latte {
namespace Layout {
class GenericLayout;
}
}

namespace Latte {
namespace Layouts {

class Storage
{

public:
    static Storage *self();
    ~Storage();

    bool isWritable(const Layout::GenericLayout *layout) const;
    bool isLatteContainment(Plasma::Containment *containment) const;
    bool isLatteContainment(const KConfigGroup &group) const;

    void lock(Layout::GenericLayout *layout) const; //! make it only read-only
    void unlock(Layout::GenericLayout *layout) const; //! make it writable which it should be the default

private:
    Storage();

private:
    QTemporaryDir m_storageTmpDir;

};

}
}

#endif
