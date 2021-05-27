/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GENERICBASICTABLE_H
#define GENERICBASICTABLE_H

// local
#include "genericdata.h"
#include "generictable.h"

// Qt
#include <QMetaType>

namespace Latte {
namespace Data {

class GenericBasicTable : public GenericTable<Generic>
{
public:
    GenericBasicTable();
    GenericBasicTable(GenericBasicTable &&o);
    GenericBasicTable(const GenericBasicTable &o);
};

}
}

Q_DECLARE_METATYPE(Latte::Data::Generic)
Q_DECLARE_METATYPE(Latte::Data::GenericBasicTable)

#endif
