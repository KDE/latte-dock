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

#ifndef SETTINGSLAYOUTSCONTROLLER_H
#define SETTINGSLAYOUTSCONTROLLER_H

// local
#include <coretypes.h>
#include "layoutsheaderview.h"
#include "layoutsmodel.h"
#include "../../lattecorona.h"
#include "../../data/layoutdata.h"
#include "../../data/layoutstable.h"

// Qt
#include <QAbstractItemModel>
#include <QHash>
#include <QSortFilterProxyModel>
#include <QTableView>


namespace Latte {
class Corona;
class CentralLayout;
class SettingsDialog;

namespace Settings {
namespace Handler {
class TabLayouts;
}
}
}

namespace Latte {
namespace Settings {
namespace Controller {

class Layouts : public QObject
{
    Q_OBJECT

public:
    explicit Layouts(Settings::Handler::TabLayouts *parent);
    ~Layouts();

    QAbstractItemModel *proxyModel() const;
    QAbstractItemModel *baseModel() const;
    QTableView *view() const;

    bool hasChangedData() const;
    bool layoutsAreChanged() const;
    bool modeIsChanged() const;

    bool inMultipleMode() const;
    void setInMultipleMode(bool inMultiple);
    void sortByColumn(int column, Qt::SortOrder order);

    bool hasSelectedLayout() const;
    const Latte::Data::Layout selectedLayoutCurrentData() const;
    const Latte::Data::Layout selectedLayoutOriginalData() const;

    void selectRow(const QString &id);
    void setLayoutProperties(const Latte::Data::Layout &layout);

    //! actions
    void reset();
    void save();
    void removeSelected();
    void toggleLockedForSelected();

    QString iconsPath() const;
    QString colorPath(const QString color) const;

    QString layoutNameForFreeActivities() const;
    void setOriginalLayoutForFreeActivities(const QString &id);

    void setOriginalInMultipleMode(const bool &inmultiple);

    void copySelectedLayout();
    const Latte::Data::Layout addLayoutForFile(QString file, QString layoutName = QString(), bool newTempDirectory = true);
    const Latte::Data::Layout addLayoutByText(QString rawLayoutText);

    //! import layouts from Latte versions <= v0.7.x
    bool importLayoutsFromV1ConfigFile(QString file);

signals:
    void dataChanged();

private slots:
    void initLayouts();
    void loadConfig();
    void saveConfig();
    void storeColumnWidths();
    void applyColumnWidths();

    void onNameDuplicatedFrom(const QString &provenId,  const QString &trialId);
    void onLayoutAddedExternally(const Data::Layout &layout);
    void onLayoutActivitiesChangedExternally(const Data::Layout &layout);

private:
    void initView();

    int rowForId(QString id) const;
    int rowForName(QString layoutName) const;

    QString uniqueTempDirectory();
    QString uniqueLayoutName(QString name);

private:
    Settings::Handler::TabLayouts *m_handler{nullptr};

    QString m_iconsPath;

    QTableView *m_view{nullptr};
    Settings::Layouts::HeaderView *m_headerView{nullptr};

    //! layoutsView ui settings
    int m_viewSortColumn;
    Qt::SortOrder m_viewSortOrder;
    QStringList m_viewColumnWidths;

    KConfigGroup m_storage;

    //! current data
    Model::Layouts *m_model{nullptr};
    QSortFilterProxyModel *m_proxyModel{nullptr};

    //! temp data
    QStringList m_tempDirectories;
};

}
}
}

#endif
