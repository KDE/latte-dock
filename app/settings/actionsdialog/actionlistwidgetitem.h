/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIONLISTWIDGETITEM_H
#define ACTIONLISTWIDGETITEM_H

// Qt
#include <QListWidgetItem>

namespace Latte {
namespace Settings {
namespace ActionsDialog {

class ActionListWidgetItem : public QListWidgetItem
{
public:
    enum ActionUserRoles
    {
        IDROLE = Qt::UserRole + 1,
        ORDERROLE
    };

    ActionListWidgetItem(const QIcon &icon, const QString &text, const int &order, const QString &id, QListWidget *parent = nullptr, int type = QListWidgetItem::Type);

    bool operator<(const QListWidgetItem &other) const override;
};

}
}
}
#endif
