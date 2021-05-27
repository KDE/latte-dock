/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef APPLETSMODEL_H
#define APPLETSMODEL_H

// local
#include "../../data/appletdata.h"

// Qt
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QList>

namespace Latte {
namespace Settings {
namespace Model {

class Applets : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum AppletsRoles
    {
        IDROLE = Qt::UserRole + 1,
        NAMEROLE,
        ICONROLE,
        SELECTEDROLE,
        SORTINGROLE,
        DESCRIPTIONROLE
    };

    enum Columns
    {
        NAMECOLUMN = 0
    };

    explicit Applets(QObject *parent);
    ~Applets();

    bool hasChangedData() const;
    bool inDefaultValues() const;

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    int row(const QString &id);

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setData(const Latte::Data::AppletsTable &applets);
    void setSelected(const Latte::Data::AppletsTable &applets);

    Latte::Data::AppletsTable selectedApplets();

    void deselectAll();
    void reset();
    void selectAll();

signals:
    void appletsDataChanged();

private:
    void initDefaults();

    void clear();

private:
    Latte::Data::AppletsTable o_applets;
    Latte::Data::AppletsTable c_applets;

    QList<QString> m_appletsWithNoPersonalData;
};

}
}
}

#endif
