/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "genericbasictable.h"

namespace Latte {
namespace Data {

GenericBasicTable::GenericBasicTable()
    : GenericTable<Generic>()
{
}

GenericBasicTable::GenericBasicTable(GenericBasicTable &&o)
    : GenericTable<Generic>(o)
{
}

GenericBasicTable::GenericBasicTable(const GenericBasicTable &o)
    : GenericTable<Generic>(o)
{
}

}
}
