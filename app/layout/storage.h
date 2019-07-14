/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef LAYOUTSTORAGE_H
#define LAYOUTSTORAGE_H

#include "genericlayout.h"

// Qt
#include <QObject>

namespace Plasma{
class Containment;
}

namespace Latte {
namespace Layout {

class Storage : public QObject
{
    Q_OBJECT

public:
    Storage(GenericLayout *parent);
    ~Storage() override;

    bool isWritable() const;
    bool isLatteContainment(Plasma::Containment *containment) const;
    bool isLatteContainment(const KConfigGroup &group) const;
    bool layoutIsBroken() const;

    void importToCorona();
    void lock(); //! make it only read-only
    void unlock(); //! make it writable which it should be the default

    void copyView(Plasma::Containment *containment);
    void syncToLayoutFile(bool removeLayoutId);

    QList<int> viewsScreens();

    /// STATIC
    //! Check if an applet config group is valid or belongs to removed applet
    static bool appletGroupIsValid(KConfigGroup appletGroup);

private:
    //! STORAGE !////
    QString availableId(QStringList all, QStringList assigned, int base);
    //! provides a new file path based the provided file. The new file
    //! has updated ids for containments and applets based on the corona
    //! loaded ones
    QString newUniqueIdsLayoutFromFile(QString file);
    //! imports a layout file and returns the containments for the docks
    QList<Plasma::Containment *> importLayoutFile(QString file);

private:
    GenericLayout *m_layout;
};

}
}

#endif
