/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutscontroller.h"

// local
#include "ui_settingsdialog.h"
#include "settingsdialog.h"
#include "tablayoutshandler.h"
#include "templateskeeper.h"
#include "delegates/activitiesdelegate.h"
#include "delegates/backgrounddelegate.h"
#include "delegates/checkboxdelegate.h"
#include "delegates/layoutnamedelegate.h"
#include "../universalsettings.h"
#include "../generic/generictools.h"
#include "../../screenpool.h"
#include "../../data/uniqueidinfo.h"
#include "../../layout/centrallayout.h"
#include "../../layouts/importer.h"
#include "../../layouts/manager.h"
#include "../../layouts/synchronizer.h"
#include "../../templates/templatesmanager.h"

// Qt
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QItemSelection>
#include <QStringList>
#include <QTemporaryDir>
#include <QTemporaryFile>

// KDE
#include <KArchive/KTar>
#include <KArchive/KArchiveEntry>
#include <KArchive/KArchiveDirectory>
#include <KMessageWidget>

namespace Latte {
namespace Settings {
namespace Controller {

Layouts::Layouts(Settings::Handler::TabLayouts *parent)
    : QObject(parent),
      m_handler(parent),
      m_model(new Model::Layouts(this, m_handler->corona())),
      m_proxyModel(new QSortFilterProxyModel(this)),
      m_view(m_handler->ui()->layoutsView),
      m_headerView(new Settings::Layouts::HeaderView(Qt::Horizontal, m_handler->dialog())),
      m_storage(KConfigGroup(KSharedConfig::openConfig(),"LatteSettingsDialog").group("TabLayouts"))
{   
    m_templatesKeeper = new Settings::Part::TemplatesKeeper(this, m_handler->corona());

    loadConfig();
    m_proxyModel->setSourceModel(m_model);

    connect(m_handler->corona()->layoutsManager()->synchronizer(), &Latte::Layouts::Synchronizer::newLayoutAdded, this, &Layouts::onLayoutAddedExternally);
    connect(m_handler->corona()->layoutsManager()->synchronizer(), &Latte::Layouts::Synchronizer::layoutActivitiesChanged, this, &Layouts::onLayoutActivitiesChangedExternally);

    connect(m_model, &QAbstractItemModel::dataChanged, this, &Layouts::dataChanged);
    connect(m_model, &Model::Layouts::rowsInserted, this, &Layouts::dataChanged);
    connect(m_model, &Model::Layouts::rowsRemoved, this, &Layouts::dataChanged);

    connect(m_model, &Model::Layouts::nameDuplicated, this, &Layouts::onNameDuplicatedFrom);

    connect(m_headerView, &QObject::destroyed, this, [&]() {
        m_viewSortColumn = m_headerView->sortIndicatorSection();
        m_viewSortOrder = m_headerView->sortIndicatorOrder();
    });

    initView();
    initLayouts();

    //! connect them after initialization of the view
    connect(m_model, &Model::Layouts::inMultipleModeChanged, this, [&]() {
        applyColumnWidths(true);
    });

    connect(m_handler->corona()->universalSettings(), &UniversalSettings::canDisableBordersChanged, this, [&]() {
        applyColumnWidths(false);
    });
}

Layouts::~Layouts()
{
    saveConfig();

    for (const auto &tempDir : m_tempDirectories) {
        QDir tDir(tempDir);

        if (tDir.exists() && tempDir.startsWith("/tmp/")) {
            tDir.removeRecursively();
        }
    }
}

QAbstractItemModel *Layouts::proxyModel() const
{
    return m_proxyModel;
}

QAbstractItemModel *Layouts::baseModel() const
{
    return m_model;
}

QTableView *Layouts::view() const
{
    return m_view;
}

Settings::Part::TemplatesKeeper *Layouts::templatesKeeper() const
{
    return m_templatesKeeper;
}

void Layouts::initView()
{
    m_view->setModel(m_proxyModel);
    m_view->setHorizontalHeader(m_headerView);
    m_view->verticalHeader()->setVisible(false);
    m_view->setSortingEnabled(true);

    m_proxyModel->setSortRole(Model::Layouts::SORTINGROLE);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_view->sortByColumn(m_viewSortColumn, m_viewSortOrder);

    //!find the available colors
    m_iconsPath = m_handler->corona()->kPackage().path() + "../../shells/org.kde.latte.shell/contents/images/canvas/";

    QDir layoutDir(m_iconsPath);
    QStringList filter;
    filter.append(QString("*print.jpg"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);
    QStringList colors;

    for (auto &file : files) {
        int colorEnd = file.lastIndexOf("print.jpg");
        QString color = file.remove(colorEnd, 9);
        colors.append(color);
    }

    m_view->setItemDelegateForColumn(Model::Layouts::NAMECOLUMN, new Settings::Layout::Delegate::LayoutName(this));
    m_view->setItemDelegateForColumn(Model::Layouts::BACKGROUNDCOLUMN, new Settings::Layout::Delegate::BackgroundDelegate(this));
    m_view->setItemDelegateForColumn(Model::Layouts::MENUCOLUMN, new Settings::Layout::Delegate::CheckBox(this));
    m_view->setItemDelegateForColumn(Model::Layouts::BORDERSCOLUMN, new Settings::Layout::Delegate::CheckBox(this));
    m_view->setItemDelegateForColumn(Model::Layouts::ACTIVITYCOLUMN, new Settings::Layout::Delegate::Activities(this));

    connect(m_view->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &Layouts::onCurrentRowChanged);

    connect(m_view, &QObject::destroyed, this, [&]() {
        storeColumnWidths(m_model->inMultipleMode());
    });
}

bool Layouts::hasChangedData() const
{
    return m_model->hasChangedData();
}

bool Layouts::layoutsAreChanged() const
{
    return m_model->layoutsAreChanged();
}

bool Layouts::modeIsChanged() const
{
    return m_model-modeIsChanged();
}

void Layouts::setOriginalInMultipleMode(const bool &inmultiple)
{
    m_model->setOriginalInMultipleMode(inmultiple);
}

bool Layouts::hasSelectedLayout() const
{
    return m_view->selectionModel()->hasSelection();
}

bool Layouts::isLayoutOriginal(const QString &currentLayoutId) const
{
    return m_model->originalLayoutsData().containsId(currentLayoutId);
}

bool Layouts::isSelectedLayoutOriginal() const
{
    if (!hasSelectedLayout()) {
        return false;
    }

    Data::Layout currentData = selectedLayoutCurrentData();

    return m_model->originalLayoutsData().containsId(currentData.id);
}

QString Layouts::colorPath(const QString color) const
{
    QString path = m_iconsPath + color + "print.jpg";

    if (!QFileInfo(path).exists()) {
        return m_iconsPath + "blueprint.jpg";
    }

    return path;
}

QString Layouts::iconsPath() const
{
    return m_iconsPath;
}

const Latte::Data::ViewsTable Layouts::selectedLayoutViews()
{
    Latte::Data::ViewsTable views;
    int selectedRow = m_view->currentIndex().row();

    if (selectedRow < 0) {
        return views;
    }

    QString selectedId = m_proxyModel->data(m_proxyModel->index(selectedRow, Model::Layouts::IDCOLUMN), Qt::UserRole).toString();
    Data::Layout selectedCurrentData = m_model->currentData(selectedId);

    Data::Layout originalSelectedData = selectedLayoutOriginalData();
    CentralLayout *central = m_handler->corona()->layoutsManager()->synchronizer()->centralLayout(originalSelectedData.name);

    bool islayoutactive{true};

    if (!central) {
        islayoutactive = false;
        central = new CentralLayout(this, selectedCurrentData.id);
    }

    selectedCurrentData.views = central->viewsTable().onlyOriginals();
    selectedCurrentData.views.isInitialized = true;

    if (!islayoutactive) {
        central->deleteLater();
    }

    return selectedCurrentData.views;
}

const Latte::Data::LayoutIcon Layouts::selectedLayoutIcon() const
{
    int selectedRow = m_view->currentIndex().row();

    if (selectedRow >= 0) {
        QString selectedId = m_proxyModel->data(m_proxyModel->index(selectedRow, Model::Layouts::IDCOLUMN), Qt::UserRole).toString();
        return m_model->currentLayoutIcon(selectedId);
    }

    return Latte::Data::LayoutIcon();

}

const Latte::Data::Layout Layouts::selectedLayoutCurrentData() const
{
    int selectedRow = m_view->currentIndex().row();
    if (selectedRow >= 0) {
        QString selectedId = m_proxyModel->data(m_proxyModel->index(selectedRow, Model::Layouts::IDCOLUMN), Qt::UserRole).toString();
        return m_model->currentData(selectedId);
    } else {
        return Latte::Data::Layout();
    }
}

const Latte::Data::Layout Layouts::selectedLayoutOriginalData() const
{
    int selectedRow = m_view->currentIndex().row();
    QString selectedId = m_proxyModel->data(m_proxyModel->index(selectedRow, Model::Layouts::IDCOLUMN), Qt::UserRole).toString();

    return m_model->originalData(selectedId);
}

const Latte::Data::Layout Layouts::currentData(const QString &currentLayoutId) const
{
    return m_model->currentData(currentLayoutId);
}

const Latte::Data::Layout Layouts::originalData(const QString &currentLayoutId) const
{
    return m_model->originalData(currentLayoutId);
}

const Latte::Data::ScreensTable Layouts::screensData()
{
    Latte::Data::ScreensTable scrtable = m_handler->corona()->screenPool()->screensTable();

    QList<int> expscreens;

    //! update activeness
    for (int i=1; i<scrtable.rowCount(); ++i) {
        scrtable[i].isActive = m_handler->corona()->screenPool()->isScreenActive(scrtable[i].id.toInt());
    }

    //! update removability based on activeness
    for (int i=0; i<scrtable.rowCount(); ++i) {
        scrtable[i].isRemovable = !scrtable[i].isActive;
    }

    //! retrieve all layouts data
    Latte::Data::LayoutsTable originalLayouts = m_model->originalLayoutsData();
    Latte::Data::LayoutsTable currentLayouts = m_model->currentLayoutsData();
    Latte::Data::LayoutsTable removedLayouts = originalLayouts.subtracted(currentLayouts);

    //! temp removed layouts should be considered because they may not be deleted in the end
    for (int i=0; i<removedLayouts.rowCount(); ++i) {
        CentralLayout *central = centralLayout(removedLayouts[i].id);

        if (!central) {
            continue;
        }

        QList<int> newexps = central->viewsExplicitScreens();

        expscreens = join(expscreens, newexps);
    }

    //! current layouts should be considered
    for (int i=0; i<currentLayouts.rowCount(); ++i) {
        CentralLayout *central = centralLayout(currentLayouts[i].id);

        if (!central) {
            continue;
        }

        QList<int> newexps = central->viewsExplicitScreens();

        expscreens = join(expscreens, newexps);
    }

    //! discovered explicit screens should be flagged as NOREMOVABLE
    for (int i=0; i<expscreens.count(); ++i) {
        QString expscridstr = QString::number(expscreens[i]);

        if (scrtable.containsId(expscridstr)) {
            scrtable[expscridstr].isRemovable = false;
        }
    }

    //! Print no-removable screens
    /*for (int i=0; i<scrtable.rowCount(); ++i) {
        if (!scrtable[i].isRemovable) {
            qDebug() <<"org.kde.latte NO REMOVABLE EXP SCREEN ::: " << scrtable[i].id;
        }
    }*/

    return scrtable;
}

bool Layouts::inMultipleMode() const
{
    return m_model->inMultipleMode();
}

void Layouts::setInMultipleMode(bool inMultiple)
{
    m_model->setInMultipleMode(inMultiple);
}

CentralLayout *Layouts::centralLayout(const QString &currentLayoutId)
{
    Data::Layout originlayoutdata = originalData(currentLayoutId);
    auto activelayout = isLayoutOriginal(currentLayoutId) ?
                m_handler->corona()->layoutsManager()->synchronizer()->centralLayout(originlayoutdata.name) : nullptr;

    Latte::CentralLayout *centrallayout = activelayout ? activelayout : new Latte::CentralLayout(this, currentLayoutId);

    return centrallayout;
}

void Layouts::applyColumnWidths(bool storeValues)
{
    bool isLastModeMultiple = !m_model->inMultipleMode();

    //! save previous values
    if (storeValues) {
        storeColumnWidths(isLastModeMultiple);
    }

    if (m_model->inMultipleMode()) {
        m_view->horizontalHeader()->setSectionResizeMode(Model::Layouts::ACTIVITYCOLUMN, QHeaderView::Stretch);
        m_view->horizontalHeader()->setSectionResizeMode(Model::Layouts::NAMECOLUMN, QHeaderView::Interactive);
    } else {
        m_view->horizontalHeader()->setSectionResizeMode(Model::Layouts::NAMECOLUMN, QHeaderView::Stretch);
        m_view->horizontalHeader()->setSectionResizeMode(Model::Layouts::ACTIVITYCOLUMN, QHeaderView::Interactive);
    }

    //! this line should be commented for debugging layouts window functionality
    m_view->setColumnHidden(Model::Layouts::IDCOLUMN, true);
    m_view->setColumnHidden(Model::Layouts::HIDDENTEXTCOLUMN, true);

    int maxColumns = Model::Layouts::ACTIVITYCOLUMN - Model::Layouts::BACKGROUNDCOLUMN; //4 - multiple

    if (m_handler->corona()->universalSettings()->canDisableBorders()) {
        m_view->setColumnHidden(Model::Layouts::BORDERSCOLUMN, false);
    } else {
        m_view->setColumnHidden(Model::Layouts::BORDERSCOLUMN, true);
    }

    if (m_model->inMultipleMode()) {
        m_view->setColumnHidden(Model::Layouts::MENUCOLUMN, true);
        m_view->setColumnHidden(Model::Layouts::ACTIVITYCOLUMN, false);
    } else {
        m_view->setColumnHidden(Model::Layouts::MENUCOLUMN, false);
        m_view->setColumnHidden(Model::Layouts::ACTIVITYCOLUMN, true);
    }

    if (!m_viewColumnWidths.isEmpty()) {
        for (int i=0; i<qMin(m_viewColumnWidths.count(), maxColumns); ++i) {
            int currentColumn = Model::Layouts::BACKGROUNDCOLUMN+i;

            if ((currentColumn == Model::Layouts::BORDERSCOLUMN && !m_handler->corona()->universalSettings()->canDisableBorders())
                    || (currentColumn == Model::Layouts::NAMECOLUMN && isLastModeMultiple)
                    || (currentColumn == Model::Layouts::MENUCOLUMN && !isLastModeMultiple)
                    || (currentColumn == Model::Layouts::ACTIVITYCOLUMN)) {
                continue;
            }

            m_view->setColumnWidth(currentColumn, m_viewColumnWidths[i].toInt());
        }
    } else {
        m_view->resizeColumnsToContents();
    }
}

int Layouts::rowForId(QString id) const
{
    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        QString rowId = m_proxyModel->data(m_proxyModel->index(i, Model::Layouts::IDCOLUMN), Qt::UserRole).toString();

        if (rowId == id) {
            return i;
        }
    }

    return -1;
}

int Layouts::rowForName(QString layoutName) const
{
    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        QString rowName = m_proxyModel->data(m_proxyModel->index(i, Model::Layouts::NAMECOLUMN), Qt::UserRole).toString();

        if (rowName == layoutName) {
            return i;
        }
    }

    return -1;
}

QString Layouts::uniqueTempDirectory()
{
    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);
    m_tempDirectories.append(tempDir.path());

    return tempDir.path();
}

QString Layouts::uniqueLayoutName(QString name)
{
    int pos_ = name.lastIndexOf(QRegExp(QString(" - [0-9]+")));

    if (m_model->containsCurrentName(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (m_model->containsCurrentName(name)) {
        name = namePart + " - " + QString::number(i);
        i++;
    }

    return name;
}

void Layouts::removeSelected()
{
    if (m_view->currentIndex().row() < 0) {
        return;
    }

    Latte::Data::Layout selectedOriginal = selectedLayoutOriginalData();

    if (m_handler->corona()->layoutsManager()->synchronizer()->layout(selectedOriginal.name)) {
        return;
    }

    int row = m_view->currentIndex().row();
    row = qMin(row, m_proxyModel->rowCount() - 1);
    m_view->selectRow(row);

    Latte::Data::Layout selected = selectedLayoutCurrentData();
    m_model->removeLayout(selected.id);
}

void Layouts::toggleEnabledForSelected()
{
    if (!hasSelectedLayout()) {
        return;
    }

    Latte::Data::Layout selected = selectedLayoutCurrentData();

    if (!selected.activities.isEmpty()) {
        m_proxyModel->setData(m_proxyModel->index(m_view->currentIndex().row(), Model::Layouts::ACTIVITYCOLUMN), QStringList(), Qt::UserRole);
    } else {
        QStringList activities;

        bool layoutsenabledonlyinspecificactivities = m_model->hasEnabledLayout()
                && !m_model->hasEnabledLayoutInAllActitivities()
                && !m_model->hasEnabledLayoutInFreeActivities();

        if (m_model->hasEnabledLayoutInCurrentActivity() || layoutsenabledonlyinspecificactivities) {
            activities << m_model->currentActivityId();
        } else if (m_model->hasEnabledLayoutInFreeActivities()) {
            activities << Data::Layout::FREEACTIVITIESID;
        } else {
            activities << Data::Layout::ALLACTIVITIESID;
        }

        m_proxyModel->setData(m_proxyModel->index(m_view->currentIndex().row(), Model::Layouts::ACTIVITYCOLUMN), activities, Qt::UserRole);
    }
}

void Layouts::toggleLockedForSelected()
{
    if (!hasSelectedLayout()) {
        return;
    }

    Latte::Data::Layout selected = selectedLayoutCurrentData();

    m_proxyModel->setData(m_proxyModel->index(m_view->currentIndex().row(), Model::Layouts::NAMECOLUMN), !selected.isLocked, Settings::Model::Layouts::ISLOCKEDROLE);
}

void Layouts::selectRow(const QString &id)
{    
    m_view->selectRow(rowForId(id));
}

void Layouts::setLayoutProperties(const Latte::Data::Layout &layout)
{
    m_model->setLayoutProperties(layout);
}

QString Layouts::layoutNameForFreeActivities() const
{
    return m_model->layoutNameForFreeActivities();
}

void Layouts::setOriginalLayoutForFreeActivities(const QString &id)
{
    m_model->setOriginalLayoutForFreeActivities(id);
    emit dataChanged();
}

void Layouts::initLayouts()
{
    m_model->clear();
    bool inMultiple{m_handler->corona()->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts};
    setInMultipleMode(inMultiple);

    m_handler->corona()->layoutsManager()->synchronizer()->updateLayoutsTable();
    Latte::Data::LayoutsTable layouts = m_handler->corona()->layoutsManager()->synchronizer()->layoutsTable();

    //! Send original loaded data to model
    m_model->setOriginalInMultipleMode(inMultiple);
    m_model->setOriginalData(layouts);

    QStringList currentLayoutNames = m_handler->corona()->layoutsManager()->currentLayoutsNames();
    if (currentLayoutNames.count() > 0) {
        m_view->selectRow(rowForName(currentLayoutNames[0]));
    }

    applyColumnWidths();

    showInitialErrorWarningMessages();
}

void Layouts::initialMessageForErroredLayouts(const int &count)
{
    if (count <= 0) {
        return;
    }

    m_handler->showInlineMessage(i18ncp("settings:counted layout with errors",
                                        "<b>Error:</b> There is <b>1 layout</b> that has reported errors.",
                                        "<b>Error:</b> There are <b>%1 layouts</b> that have reported errors.",
                                        count),
                                     KMessageWidget::Error);
}

void Layouts::initialMessageForWarningLayouts(const int &count)
{
    if (count <= 0) {
        return;
    }

    m_handler->showInlineMessage(i18ncp("settings:counted layout with warnings",
                                        "<b>Warning:</b> There is <b>1 layout</b> that has reported warnings.",
                                        "<b>Warning:</b> There are <b>%1 layouts</b> that have reported warnings.",
                                        count),
                                     KMessageWidget::Warning);
}


void Layouts::messageForErroredLayout(const Data::Layout &layout)
{
    //! add actions
    QAction *examineaction = new QAction(i18n("Examine..."), this);
    examineaction->setIcon(QIcon::fromTheme("document-preview"));
    examineaction->setData(layout.id);
    QList<QAction *> actions;
    actions << examineaction;

    connect(examineaction, &QAction::triggered, this, [&, examineaction]() {
        QString currentid = examineaction->data().toString();

        if (!currentid.isEmpty()) {
            selectRow(currentid);
            m_handler->showViewsDialog();
        }
    });

    if (!layout.hasErrors() && layout.hasWarnings()) {
        //! add only warnings first
        m_handler->showInlineMessage(i18ncp("settings:named layout with warnings",
                                            "<b>Warning: %2</b> layout has reported <b>1 warning</b> that need your attention.",
                                            "<b>Warning: %2</b> layout has reported <b>%1 warnings</b> that need your attention.",
                                            layout.warnings,
                                            layout.name),
                                     KMessageWidget::Warning,
                                     false,
                                     actions);
    } else if (layout.hasErrors() && !layout.hasWarnings()) {
        //! add errors in the end in order to be read by the user
        m_handler->showInlineMessage(i18nc("settings:named layout with errors",
                                           "<b>Error: %2</b> layout has reported <b>1 error</b> that you need to repair.",
                                           "<b>Error: %2</b> layout has reported <b>%1 errors</b> that you need to repair.",
                                           layout.errors,
                                           layout.name),
                                     KMessageWidget::Error,
                                     true,
                                     actions);
    } else if (layout.hasErrors() && layout.hasWarnings()) {
        //! add most important errors in the end in order to be read by the user
        QString errorstr = i18ncp("errors count",
                                 "1 error",
                                 "%1 errors",
                                 layout.errors);
        QString warningstr = i18ncp("warnings count",
                                   "1 warning",
                                   "%1 warnings",
                                   layout.warnings);

        m_handler->showInlineMessage(i18nc("settings: named layout with %2 errors and %3 warnings",
                                           "<b>Error: %1</b> layout has reported <b>%2</b> and <b>%3</b> that you need to repair.",
                                           layout.name,
                                           errorstr,
                                           warningstr),
                                     KMessageWidget::Error,
                                     true,
                                     actions);
    }
}

void Layouts::showInitialErrorWarningMessages()
{
    if (!m_hasShownInitialErrorWarningMessages) {
        m_hasShownInitialErrorWarningMessages = true;

        Latte::Data::LayoutsTable layouts = m_handler->corona()->layoutsManager()->synchronizer()->layoutsTable();

        int erroredlayouts{0};
        int warninglayouts{0};

        for (int i=0; i<layouts.rowCount(); ++i) {
            if (layouts[i].hasErrors()) {
                erroredlayouts++;
            } else if (layouts[i].hasWarnings()) {
                warninglayouts++;
            }
        }

        onCurrentRowChanged();
        initialMessageForWarningLayouts(warninglayouts);
        initialMessageForErroredLayouts(erroredlayouts);
    }
}

void Layouts::onCurrentRowChanged()
{
    if (!hasSelectedLayout()) {
        return;
    }

    if (!m_handler->isViewsDialogVisible()) {
        Latte::Data::Layout selectedlayout = selectedLayoutCurrentData();

        if (selectedlayout.hasErrors() || selectedlayout.hasWarnings()) {
            messageForErroredLayout(selectedlayout);
        }
    }
}

void Layouts::onLayoutActivitiesChangedExternally(const Data::Layout &layout)
{
    m_model->setOriginalActivitiesForLayout(layout);
}

void Layouts::onLayoutAddedExternally(const Data::Layout &layout)
{
    m_model->appendOriginalLayout(layout);
}

void Layouts::setLayoutCurrentErrorsWarnings(const QString &layoutCurrentId, const int &errors, const int &warnings)
{
    Latte::Data::Layout layout = m_model->currentData(layoutCurrentId);

    if (!layout.isNull()) {
        layout.errors = errors;
        layout.warnings = warnings;
        setLayoutProperties(layout);
    }

}

void Layouts::sortByColumn(int column, Qt::SortOrder order)
{
    m_view->sortByColumn(column, order);
}

const Latte::Data::Layout Layouts::addLayoutForFile(QString file, QString layoutName, bool newTempDirectory)
{
    if (layoutName.isEmpty()) {
        layoutName = CentralLayout::layoutName(file);
    }

    layoutName = uniqueLayoutName(layoutName);

    Latte::Data::Layout copied;

    if (newTempDirectory) {
        copied.id = uniqueTempDirectory() + "/" + layoutName + ".layout.latte";
        QFile(file).copy(copied.id);
    } else {
        copied.id = file;
    }

    QFileInfo newFileInfo(copied.id);

    if (newFileInfo.exists() && !newFileInfo.isWritable()) {
        QFile(copied.id).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }

    CentralLayout *settings = new CentralLayout(this, copied.id);

    copied = settings->data();
    copied.name = uniqueLayoutName(layoutName);

    m_model->appendLayout(copied);
    m_view->selectRow(rowForId(copied.id));

    return copied;
}

const Latte::Data::Layout Layouts::addLayoutByText(QString rawLayoutText)
{
    QTemporaryFile tempFile;
    tempFile.open();
    QTextStream stream(&tempFile);
    stream << rawLayoutText;
    stream.flush();
    tempFile.close();

    Latte::Data::Layout newLayout = addLayoutForFile(tempFile.fileName(),i18n("Dropped Raw Layout"));

    int selectedRow = m_view->currentIndex().row();
    QModelIndex tIndex = m_proxyModel->index(selectedRow, Model::Layouts::NAMECOLUMN);
    m_view->edit(tIndex);
    
    /**Window has to be activated explicitely since the window where the drag
     * started would otherwise be the active window. By activating the window
       the user can immediately change the name by simply typing.*/
    m_handler->dialog()->activateWindow();
    
    return newLayout;
}

void Layouts::duplicateSelectedLayout()
{
    int row = m_view->currentIndex().row();

    if (row < 0) {
        return;
    }

    Latte::Data::Layout selectedLayoutCurrent = selectedLayoutCurrentData();
    Latte::Data::Layout selectedLayoutOriginal = selectedLayoutOriginalData();
    selectedLayoutOriginal = selectedLayoutOriginal.isEmpty() ? selectedLayoutCurrent : selectedLayoutOriginal;


    //! Update original layout before duplicating if this layout is active
    if (m_handler->corona()->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
        Latte::CentralLayout *central = m_handler->corona()->layoutsManager()->synchronizer()->centralLayout(selectedLayoutOriginal.name);
        if (central) {
            central->syncToLayoutFile();
        }
    }

    Latte::Data::Layout copied = selectedLayoutCurrent;

    copied.name = uniqueLayoutName(selectedLayoutCurrent.name);
    copied.id = uniqueTempDirectory() + "/" + copied.name + ".layout.latte";;
    copied.isActive = false;
    copied.isConsideredActive = false;
    copied.isLocked = false;
    copied.activities = QStringList();

    QFile(selectedLayoutCurrent.id).copy(copied.id);
    QFileInfo newFileInfo(copied.id);

    if (newFileInfo.exists() && !newFileInfo.isWritable()) {
        QFile(copied.id).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }

    CentralLayout *settings = new CentralLayout(this, copied.id);
    settings->clearLastUsedActivity();
    settings->setActivities(QStringList());

    m_model->appendLayout(copied);

    m_view->selectRow(rowForId(copied.id));
}

bool Layouts::importLayoutsFromV1ConfigFile(QString file)
{
    KTar archive(file, QStringLiteral("application/x-tar"));
    archive.open(QIODevice::ReadOnly);

    //! if the file isnt a tar archive
    if (archive.isOpen()) {
        QDir tempDir{uniqueTempDirectory()};

        const auto archiveRootDir = archive.directory();

        for (const auto &name : archiveRootDir->entries()) {
            auto fileEntry = archiveRootDir->file(name);
            fileEntry->copyTo(tempDir.absolutePath());
        }

        QString name = Latte::Layouts::Importer::nameOfConfigFile(file);

        QString applets(tempDir.absolutePath() + "/" + "lattedock-appletsrc");

        if (QFile(applets).exists()) {
            QStringList importedlayouts;

            if (m_handler->corona()->layoutsManager()->importer()->importOldLayout(applets, name, false, tempDir.absolutePath())) {
                Latte::Data::Layout imported = addLayoutForFile(tempDir.absolutePath() + "/" + name + ".layout.latte", name);
                importedlayouts << imported.name;
            }

            QString alternativeName = name + "-" + i18nc("layout", "Alternative");

            if (m_handler->corona()->layoutsManager()->importer()->importOldLayout(applets, alternativeName, false, tempDir.absolutePath())) {
                Latte::Data::Layout imported = addLayoutForFile(tempDir.absolutePath() + "/" + alternativeName + ".layout.latte", alternativeName, false);
                importedlayouts << imported.name;
            }

            if (importedlayouts.count() > 0) {
                m_handler->showInlineMessage(i18np("Layout <b>%2</b> imported successfully...",
                                                   "Layouts <b>%2</b> imported successfully...",
                                                   importedlayouts.count(),
                                                   importedlayouts.join(",")),
                        KMessageWidget::Positive);

                return true;
            }
        }
    }

    return false;
}

void Layouts::reset()
{
    m_model->resetData();
    QStringList currentLayoutNames = m_handler->corona()->layoutsManager()->currentLayoutsNames();
    if (currentLayoutNames.count() > 0) {
        m_view->selectRow(rowForName(currentLayoutNames[0]));
    }

    //! Clear any templates keeper data
    m_templatesKeeper->clear();
}

void Layouts::save()
{
    //! Update Layouts
    QStringList knownActivities = m_handler->corona()->layoutsManager()->synchronizer()->activities();

    QTemporaryDir layoutTempDir;

    qDebug() << "Temporary Directory ::: " << layoutTempDir.path();

    QString switchToLayout;

    QHash<QString, Latte::CentralLayout *> activeLayoutsToRename;

    Latte::Data::LayoutsTable originalLayouts = m_model->originalLayoutsData();
    Latte::Data::LayoutsTable currentLayouts = m_model->currentLayoutsData();
    Latte::Data::LayoutsTable removedLayouts = originalLayouts.subtracted(currentLayouts);

    //! remove layouts that have been removed from the user
    for (int i=0; i<removedLayouts.rowCount(); ++i) {
        QFile(removedLayouts[i].id).remove();
    }

    QList<Data::UniqueIdInfo> alteredIdsInfo;

    QList<Latte::Data::Layout> alteredLayouts = m_model->alteredLayouts();

    for (int i = 0; i < alteredLayouts.count(); ++i) {
        Latte::Data::Layout iLayoutCurrentData = alteredLayouts[i];
        Latte::Data::Layout iLayoutOriginalData = m_model->originalData(iLayoutCurrentData.id);
        iLayoutOriginalData = iLayoutOriginalData.isEmpty() ? iLayoutCurrentData : iLayoutOriginalData;

        //qDebug() << i << ". " << id << " - " << color << " - " << name << " - " << menu << " - " << lActivities;
        //! update the generic parts of the layouts
        bool isOriginalLayout = m_model->originalLayoutsData().containsId(iLayoutCurrentData.id);
        Latte::CentralLayout *centralActive= isOriginalLayout ? m_handler->corona()->layoutsManager()->synchronizer()->centralLayout(iLayoutOriginalData.name) : nullptr;
        Latte::CentralLayout *central = centralActive ? centralActive : new Latte::CentralLayout(this, iLayoutCurrentData.id);

        //! unlock read-only layout
        if (!central->isWritable()) {
            central->unlock();
        }

        //! Icon
        central->setIcon(iLayoutCurrentData.icon);

        //! Custom Scheme
        central->setSchemeFile(iLayoutCurrentData.schemeFile);

        //! Backgrounds
        central->setBackgroundStyle(iLayoutCurrentData.backgroundStyle);
        central->setColor(iLayoutCurrentData.color);
        central->setCustomBackground(iLayoutCurrentData.background);
        central->setCustomTextColor(iLayoutCurrentData.textColor);

        //! Extra Properties
        central->setShowInMenu(iLayoutCurrentData.isShownInMenu);
        central->setDisableBordersForMaximizedWindows(iLayoutCurrentData.hasDisabledBorders);
        central->setPopUpMargin(iLayoutCurrentData.popUpMargin);
        central->setActivities(iLayoutCurrentData.activities);

        //! If the layout name changed OR the layout path is a temporary one
        if ((iLayoutCurrentData.name != iLayoutOriginalData.name) || iLayoutCurrentData.isTemporary()) {
            //! If the layout is Active in MultipleLayouts
            if (central->isActive()) {
                qDebug() << " Active Layout Should Be Renamed From : " << central->name() << " TO :: " << iLayoutCurrentData.name;
                activeLayoutsToRename[iLayoutCurrentData.name] = central;
            }

            QString tempFile = layoutTempDir.path() + "/" + QString(central->name() + ".layout.latte");
            qDebug() << "new temp file ::: " << tempFile;

            QFile(iLayoutCurrentData.id).rename(tempFile);

            Data::UniqueIdInfo idInfo;

            idInfo.oldId = iLayoutCurrentData.id;
            idInfo.newId = tempFile;
            idInfo.newName = iLayoutCurrentData.name;

            alteredIdsInfo << idInfo;
        }
    }

    //! this is necessary in case two layouts have to swap names
    //! so we copy first the layouts in a temp directory and afterwards all
    //! together we move them in the official layout directory
    for (int i = 0; i < alteredIdsInfo.count(); ++i) {
        Data::UniqueIdInfo idInfo = alteredIdsInfo[i];

        QString newFile = Latte::Layouts::Importer::layoutUserFilePath(idInfo.newName);
        QFile(idInfo.newId).rename(newFile);


        //! updating the #SETTINGSID in the model for the layout that was renamed
        for (int j = 0; j < m_model->rowCount(); ++j) {
            Latte::Data::Layout jLayout = m_model->at(j);

            if (jLayout.id == idInfo.oldId) {
                m_model->setData(m_model->index(j, Model::Layouts::IDCOLUMN), newFile, Qt::UserRole);
            }
        }
    }

    if (m_handler->corona()->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
        for (const auto &newLayoutName : activeLayoutsToRename.keys()) {
            Latte::CentralLayout *layoutPtr = activeLayoutsToRename[newLayoutName];
            qDebug() << " Active Layout of Type: " << layoutPtr->type() << " Is Renamed From : " << activeLayoutsToRename[newLayoutName]->name() << " TO :: " << newLayoutName;
            layoutPtr->renameLayout(newLayoutName);
        }
    }

    //! lock layouts in the end when the user has chosen it
    for (int i = 0; i < alteredLayouts.count(); ++i) {
        Latte::Data::Layout layoutCurrentData = alteredLayouts[i];
        Latte::Data::Layout layoutOriginalData = m_model->originalData(layoutCurrentData.id);
        layoutOriginalData = layoutOriginalData.isEmpty() ? layoutCurrentData : layoutOriginalData;

        Latte::CentralLayout *layoutPtr = m_handler->corona()->layoutsManager()->synchronizer()->centralLayout(layoutOriginalData.name);

        if (!layoutPtr) {
            layoutPtr = new Latte::CentralLayout(this, layoutCurrentData.id);
        }

        if (layoutCurrentData.isLocked && layoutPtr && layoutPtr->isWritable()) {
            layoutPtr->lock();
        }
    }

    //! send new layouts data in layoutsmanager
    m_handler->corona()->layoutsManager()->synchronizer()->setLayoutsTable(currentLayouts);

    //! make sure that there is a layout for free activities
    //! send to layout manager in which layout to switch
    MemoryUsage::LayoutsMemory inMemoryOption = inMultipleMode() ? Latte::MemoryUsage::MultipleLayouts : Latte::MemoryUsage::SingleLayout;

    if (inMemoryOption == MemoryUsage::SingleLayout) {
        bool inrenamingsingleactivelayout = (activeLayoutsToRename.count() > 0);
        QString currentSingleLayoutName = m_handler->corona()->universalSettings()->singleModeLayoutName();
        QString nextSingleLayoutName = inrenamingsingleactivelayout ? activeLayoutsToRename.keys()[0] : currentSingleLayoutName;

        if (inrenamingsingleactivelayout && !currentLayouts.containsName(currentSingleLayoutName)) {
            m_handler->corona()->layoutsManager()->synchronizer()->setIsSingleLayoutInDeprecatedRenaming(true);
        }

        m_handler->corona()->layoutsManager()->switchToLayout(nextSingleLayoutName, MemoryUsage::SingleLayout);
    }  else {
        m_handler->corona()->layoutsManager()->switchToLayout("", MemoryUsage::MultipleLayouts);
    }

    m_model->applyData();

    //! Clear any templates keeper data
    m_templatesKeeper->clear();

    emit dataChanged();
}

void Layouts::storeColumnWidths(bool inMultipleMode)
{   
    if (m_viewColumnWidths.isEmpty()) {
        m_viewColumnWidths << "" << "" << "" << "";
    }

    m_viewColumnWidths[0] = QString::number(m_view->columnWidth(Model::Layouts::BACKGROUNDCOLUMN));

    if (inMultipleMode) {
        m_viewColumnWidths[1] = QString::number(m_view->columnWidth(Model::Layouts::NAMECOLUMN));
    }

    if (!inMultipleMode) {
        m_viewColumnWidths[2] = QString::number(m_view->columnWidth(Model::Layouts::MENUCOLUMN));
    }

    if (m_handler->corona()->universalSettings()->canDisableBorders()) {
        m_viewColumnWidths[3] = QString::number(m_view->columnWidth(Model::Layouts::BORDERSCOLUMN));
    }
}

void Layouts::onNameDuplicatedFrom(const QString &provenId, const QString &trialId)
{
    //! duplicated layout name
    int pRow = rowForId(provenId);
    int tRow = rowForId(trialId);

    int originalRow = m_model->rowForId(provenId);
    Latte::Data::Layout provenLayout = m_model->at(originalRow);

    m_handler->showInlineMessage(i18nc("settings: layout name used","Layout <b>%1</b> is already used, please provide a different name...", provenLayout.name),
                                 KMessageWidget::Error);

    QModelIndex tIndex = m_proxyModel->index(tRow, Model::Layouts::NAMECOLUMN);

    //! avoid losing focuse
    QTimer::singleShot(0, [this, tIndex]() {
        m_view->edit(tIndex);
    });
}

QList<int> Layouts::join(const QList<int> &currentRecords, const QList<int> &newRecords)
{
    QList<int> result = currentRecords;

    for(int i=0; i<newRecords.count(); ++i) {
        if (!result.contains(newRecords[i])) {
            result << newRecords[i];
        }
    }

    return result;
}

void Layouts::loadConfig()
{
    QStringList defaultcolumnwidths;
    defaultcolumnwidths << QString::number(48) << QString::number(508) << QString::number(129) << QString::number(104);

    //! new storage
    m_viewColumnWidths = m_storage.readEntry("columnWidths", defaultcolumnwidths);
    m_viewSortColumn = m_storage.readEntry("sortColumn", (int)Model::Layouts::NAMECOLUMN);
    m_viewSortOrder = static_cast<Qt::SortOrder>(m_storage.readEntry("sortOrder", (int)Qt::AscendingOrder));
}

void Layouts::saveConfig()
{
    m_storage.writeEntry("columnWidths", m_viewColumnWidths);
    m_storage.writeEntry("sortColumn", m_viewSortColumn);
    m_storage.writeEntry("sortOrder", (int)m_viewSortOrder);
}

}
}
}
