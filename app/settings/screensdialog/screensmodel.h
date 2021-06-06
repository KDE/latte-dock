/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCREENSMODEL_H
#define SCREENSMODEL_H

// local
#include "../../data/screendata.h"

// Qt
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>

namespace Latte {
namespace Settings {
namespace Model {

class Screens : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ScreensRoles
    {
        IDROLE = Qt::UserRole + 1,
        ISSCREENACTIVEROLE,
        ISSELECTEDROLE,
        SCREENDATAROLE,
        SORTINGROLE
    };

    enum Columns
    {
        SCREENCOLUMN = 0,
        LASTCOLUMN
    };

    enum SortingPriority
    {
        NORMALPRIORITY = 10,
        MEDIUMPRIORITY = 100,
        HIGHPRIORITY = 1000,
        HIGHESTPRIORITY = 10000
    };

    explicit Screens(QObject *parent);
    ~Screens();

    bool hasChangedData() const;
    bool hasChecked() const;
    bool inDefaultValues() const;

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    int row(const QString &id);

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setData(const Latte::Data::ScreensTable &screens);
    void setSelected(const Latte::Data::ScreensTable &screens);

    Latte::Data::ScreensTable checkedScreens();

    void deselectAll();
    void reset();    

signals:
    void screenDataChanged();

private:
    void initDefaults();

    void clear();

    QString sortableId(const QString &id) const;
    QString sortableText(const int &priority, const QString &text) const;

private:
    Latte::Data::ScreensTable o_screens;
    Latte::Data::ScreensTable c_screens;

};

}
}
}

#endif
