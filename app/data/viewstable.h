/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef VIEWSTABLEDATA_H
#define VIEWSTABLEDATA_H

// local
#include "generictable.h"
#include "viewdata.h"

// Qt
#include <QList>

namespace Latte {
namespace Data {

class ViewsTable : public GenericTable<View>
{

public:
    ViewsTable();
    ViewsTable(ViewsTable &&o);
    ViewsTable(const ViewsTable &o);

    bool isInitialized{false};

    void print();

    void appendTemporaryView(const Data::View &view);

    bool hasContainmentId(const QString &cid) const;

    //! Operators
    ViewsTable &operator=(const ViewsTable &rhs);
    ViewsTable &operator=(ViewsTable &&rhs);
    bool operator==(const ViewsTable &rhs) const;
    bool operator!=(const ViewsTable &rhs) const;
    ViewsTable subtracted(const ViewsTable &rhs) const;
};

}
}

Q_DECLARE_METATYPE(Latte::Data::ViewsTable)

#endif
