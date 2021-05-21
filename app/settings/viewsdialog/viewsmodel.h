
/*
 * SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef VIEWSMODEL_H
#define VIEWSMODEL_H

// local
#include "../../lattecorona.h"
#include "../../data/genericbasictable.h"
#include "../../data/screendata.h"
#include "../../data/viewdata.h"
#include "../../data/viewstable.h"
#include "../../layout/centrallayout.h"

// Qt
#include <QAbstractTableModel>
#include <QModelIndex>

namespace Latte {
namespace Settings {
namespace Model {

class Views : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        IDCOLUMN = 0,
        NAMECOLUMN,
        SCREENCOLUMN,
        EDGECOLUMN,
        ALIGNMENTCOLUMN,
        SUBCONTAINMENTSCOLUMN,
        LASTCOLUMN
    };

    enum Roles
    {
        IDROLE = Qt::UserRole + 1,
        NAMEROLE,
        ISACTIVEROLE,
        ISCHANGEDROLE,
        HASCHANGEDVIEWROLE,
        CHOICESROLE,
        SCREENROLE,
        VIEWROLE,
        ISMOVEORIGINROLE,
        ERRORSROLE,
        WARNINGSROLE,
        SORTINGROLE
    };

    enum SortingPriority
    {
        NORMALPRIORITY = 10,
        MEDIUMPRIORITY = 100,
        HIGHPRIORITY = 1000,
        HIGHESTPRIORITY = 10000
    };

    explicit Views(QObject *parent, Latte::Corona *corona);
    ~Views();

    bool hasChangedData() const;
    bool containsCurrentName(const QString &name) const;

    //! all original data will become also current
    void resetData();

    void appendTemporaryView(const Latte::Data::View &view);
    void removeView(const QString &id);

    int rowCount() const;
    static int columnCount();
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    int rowForId(const QString &id) const;

    QString viewForSubContainment(const QString &sid);

    const Latte::Data::View &at(const int &row);
    const Latte::Data::View currentData(const QString &id);
    const Latte::Data::View originalData(const QString &id);

    const Latte::Data::ViewsTable &currentViewsData();
    const Latte::Data::ViewsTable &originalViewsData();

    void setOriginalData(Latte::Data::ViewsTable &data);
    void setOriginalView(QString currentViewId, Latte::Data::View &view);
    void updateCurrentView(QString currentViewId, Latte::Data::View &view);
    void clearErrorsAndWarnings();

    void updateActiveStatesBasedOn(const CentralLayout *layout);

    Latte::Data::ViewsTable alteredViews() const;
    Latte::Data::ViewsTable newViews() const;

signals:
    void rowsInserted();
    void rowsRemoved();

private slots:
    void clear();

    void initEdges();
    void initAlignments();
    void populateScreens();

private:
    Data::ViewsTable edgesChoices(const Data::View &view) const;
    Data::ViewsTable horizontalAlignmentChoices(const Data::View &view) const;
    Data::ViewsTable verticalAlignmentChoices(const Data::View &view) const;

    bool isVertical(const Plasma::Types::Location &location) const;

    int sortingFactorForState(const Data::View &view) const;
    int sortingFactorForScreen(const Data::View &view) const;
    int sortingFactorForEdge(const Data::View &view) const;
    int sortingFactorForAlignment(const Data::View &view) const;
    int sortingFactorForSubContainments(const Data::View &view) const;

    //! based on priority a sortable text is returned
    QString sortableText(const int &priority, const QString &text) const;

    Latte::Data::Screen screenData(const QString &viewId) const;

private:
    Latte::Data::ViewsTable m_viewsTable;
    Latte::Data::ViewsTable o_viewsTable;

    Latte::Corona *m_corona{nullptr};

    Data::ViewsTable s_edges;
    Data::ViewsTable s_horizontalAlignments;
    Data::ViewsTable s_verticalAlignments;
    Latte::Data::ScreensTable s_screens;
};

}
}
}

#endif
