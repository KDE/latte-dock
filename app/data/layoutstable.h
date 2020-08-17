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

#ifndef LAYOUTSTABLEDATA_H
#define LAYOUTSTABLEDATA_H

// local
#include "generictable.h"
#include "layoutdata.h"
#include "../layouts/synchronizer.h"

// Qt
#include <QList>

namespace Latte {
namespace Data {

class LayoutsTable : public GenericTable<Layout>
{

public:
    LayoutsTable();
    LayoutsTable(LayoutsTable &&o);
    LayoutsTable(const LayoutsTable &o);

    //! Operators
    LayoutsTable &operator=(const LayoutsTable &rhs);
    LayoutsTable &operator=(LayoutsTable &&rhs);
    LayoutsTable subtracted(const LayoutsTable &rhs) const;

    QStringList allSharesIds() const;
    QStringList allSharesNames() const;
    Latte::Layouts::SharesMap sharesMap() const;

    void setLayoutForFreeActivities(const QString &id);
};

}
}

Q_DECLARE_METATYPE(Latte::Data::LayoutsTable)

#endif
