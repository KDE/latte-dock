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
    bool isBroken(const Layout::GenericLayout *layout, QStringList &errors) const;

    void lock(const Layout::GenericLayout *layout); //! make it only read-only
    void unlock(const Layout::GenericLayout *layout); //! make it writable which it should be the default

    void importToCorona(const Layout::GenericLayout *layout);
    void syncToLayoutFile(const Layout::GenericLayout *layout, bool removeLayoutId);

    /// STATIC
    //! Check if an applet config group is valid or belongs to removed applet
    static bool appletGroupIsValid(const KConfigGroup &appletGroup);

private:
    Storage();

    //! STORAGE !////
    QString availableId(QStringList all, QStringList assigned, int base);
    //! provides a new file path based the provided file. The new file
    //! has updated ids for containments and applets based on the corona
    //! loaded ones
    QString newUniqueIdsLayoutFromFile(const Layout::GenericLayout *layout, QString file);
    //! imports a layout file and returns the containments for the docks
    QList<Plasma::Containment *> importLayoutFile(const Layout::GenericLayout *layout, QString file);

private:
    QTemporaryDir m_storageTmpDir;

};

}
}

#endif
