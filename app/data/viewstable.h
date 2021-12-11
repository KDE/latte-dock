/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    ViewsTable onlyOriginals() const;
};

}
}

Q_DECLARE_METATYPE(Latte::Data::ViewsTable)

#endif
