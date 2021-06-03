/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
#include "../../data/screendata.h"
#include "../../layout/centrallayout.h"

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
namespace Part{
class TemplatesKeeper;
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
    Settings::Part::TemplatesKeeper *templatesKeeper() const;

    bool hasChangedData() const;
    bool layoutsAreChanged() const;
    bool modeIsChanged() const;

    bool inMultipleMode() const;
    void setInMultipleMode(bool inMultiple);
    void sortByColumn(int column, Qt::SortOrder order);

    bool hasSelectedLayout() const;
    bool isSelectedLayoutOriginal() const;
    bool isLayoutOriginal(const QString &currentLayoutId) const;
    const Latte::Data::Layout selectedLayoutCurrentData() const;
    const Latte::Data::Layout selectedLayoutOriginalData() const;
    const Latte::Data::LayoutIcon selectedLayoutIcon() const;
    const Latte::Data::ViewsTable selectedLayoutViews();

    const Latte::Data::Layout currentData(const QString &currentLayoutId) const;
    const Latte::Data::Layout originalData(const QString &currentLayoutId) const;

    void selectRow(const QString &id);
    void setLayoutProperties(const Latte::Data::Layout &layout);

    //! actions
    void reset();
    void save();
    void removeSelected();
    void toggleEnabledForSelected();
    void toggleLockedForSelected();    

    QString iconsPath() const;
    QString colorPath(const QString color) const;

    QString layoutNameForFreeActivities() const;
    void setOriginalLayoutForFreeActivities(const QString &id);

    void setOriginalInMultipleMode(const bool &inmultiple);
    void setLayoutCurrentErrorsWarnings(const QString &layoutCurrentId, const int &errors, const int &warnings);

    void duplicateSelectedLayout();
    const Latte::Data::Layout addLayoutForFile(QString file, QString layoutName = QString(), bool newTempDirectory = true);
    const Latte::Data::Layout addLayoutByText(QString rawLayoutText);

    CentralLayout *centralLayout(const QString &currentLayoutId);

    const Latte::Data::ScreensTable screensData();

    //! import layouts from Latte versions <= v0.7.x
    bool importLayoutsFromV1ConfigFile(QString file);

signals:
    void dataChanged();

private slots:
    void initLayouts();
    void loadConfig();
    void saveConfig();
    void storeColumnWidths(bool inMultipleMode);
    void applyColumnWidths(bool storeValues = false);

    void showInitialErrorWarningMessages();

    void onCurrentRowChanged();
    void onNameDuplicatedFrom(const QString &provenId,  const QString &trialId);
    void onLayoutAddedExternally(const Data::Layout &layout);
    void onLayoutActivitiesChangedExternally(const Data::Layout &layout);  

private:
    void initView();

    int rowForId(QString id) const;
    int rowForName(QString layoutName) const;

    QString uniqueTempDirectory();
    QString uniqueLayoutName(QString name);

    QList<int> join(const QList<int> &currentRecords, const QList<int> &newRecords);

    void initialMessageForErroredLayouts(const int &count);
    void initialMessageForWarningLayouts(const int &count);
    void messageForErroredLayout(const Data::Layout &layout);

private:
    Settings::Handler::TabLayouts *m_handler{nullptr};
    Settings::Part::TemplatesKeeper *m_templatesKeeper{nullptr};

    bool m_hasShownInitialErrorWarningMessages{false};

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
