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

#include "layoutscontroller.h"

// local
#include "../universalsettings.h"
#include "../delegates/activitiesdelegate.h"
#include "../delegates/backgroundcmbdelegate.h"
#include "../delegates/checkboxdelegate.h"
#include "../delegates/layoutnamedelegate.h"
#include "../delegates/shareddelegate.h"
#include "../tools/settingstools.h"
#include "../../layout/genericlayout.h"
#include "../../layout/centrallayout.h"
#include "../../layout/sharedlayout.h"
#include "../../layouts/importer.h"
#include "../../layouts/manager.h"
#include "../../layouts/synchronizer.h"

// Qt
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QMessageBox>
#include <QStringList>
#include <QTemporaryDir>

// KDE
#include <KActivities/Controller>
#include <KArchive/KTar>
#include <KArchive/KArchiveEntry>
#include <KArchive/KArchiveDirectory>
#include <KNotification>


namespace Latte {
namespace Settings {
namespace Controller {

Layouts::Layouts(QDialog *parent, Latte::Corona *corona, QTableView *view)
    : QObject(parent),
      m_parent(parent),
      m_corona(corona),
      m_model(new Model::Layouts(this, corona)),
      m_view(view)
{
    setOriginalInMultipleMode(m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts);

    initView();
    loadLayouts();

    connect(m_model, &QAbstractItemModel::dataChanged, this, &Layouts::dataChanged);
}

Layouts::~Layouts()
{
    //! remove
    qDeleteAll(m_layouts);

    if (m_model) {
        delete m_model;
    }

    for (const auto &tempDir : m_tempDirectories) {
        QDir tDir(tempDir);

        if (tDir.exists() && tempDir.startsWith("/tmp/")) {
            tDir.removeRecursively();
        }
    }
}

Model::Layouts *Layouts::model() const
{
    return m_model;
}

QTableView *Layouts::view() const
{
    return m_view;
}

void Layouts::initView()
{
    m_view->setModel(m_model);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->verticalHeader()->setVisible(false);

    //!find the available colors
    QString iconsPath(m_corona->kPackage().path() + "../../plasmoids/org.kde.latte.containment/contents/icons/");
    QDir layoutDir(iconsPath);
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
    m_view->setItemDelegateForColumn(Model::Layouts::BACKGROUNDCOLUMN, new Settings::Layout::Delegate::BackgroundCmbBox(this, iconsPath, colors));
    m_view->setItemDelegateForColumn(Model::Layouts::MENUCOLUMN, new Settings::Layout::Delegate::CheckBox(this));
    m_view->setItemDelegateForColumn(Model::Layouts::BORDERSCOLUMN, new Settings::Layout::Delegate::CheckBox(this));
    m_view->setItemDelegateForColumn(Model::Layouts::ACTIVITYCOLUMN, new Settings::Layout::Delegate::Activities(this));
    m_view->setItemDelegateForColumn(Model::Layouts::SHAREDCOLUMN, new Settings::Layout::Delegate::Shared(this));

    connect(m_view, &QObject::destroyed, this, &Controller::Layouts::saveColumnWidths);

    //! update all layouts view when runningActivities changed. This way we update immediately
    //! the running Activities in Activities checkboxes which are shown as bold
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged,
            this, [&]() {
        m_view->update();
    });
}

bool Layouts::dataAreChanged() const
{
    return ((o_originalInMultipleMode != m_model->inMultipleMode())
            || (o_layoutsOriginalData != m_model->currentData()));
}

bool Layouts::hasSelectedLayout() const
{
    int selectedRow = m_view->currentIndex().row();

    return (selectedRow >= 0);
}

bool Layouts::selectedLayoutIsCurrentActive() const
{
    Data::Layout selected = selectedLayout();

    return (selected.isActive && (selected.originalName() == m_corona->layoutsManager()->synchronizer()->currentLayoutName()));
}

const Data::Layout &Layouts::selectedLayout() const
{
    int selectedRow = m_view->currentIndex().row();

    return m_model->at(selectedRow);
}

bool Layouts::inMultipleMode() const
{
    return m_model->inMultipleMode();
}

void Layouts::setInMultipleMode(bool inMultiple)
{
    m_model->setInMultipleMode(inMultiple);

    if (inMultiple) {
        m_view->setColumnHidden(Model::Layouts::SHAREDCOLUMN, false);

        //! column widths
        QStringList cWidths = m_corona->universalSettings()->layoutsColumnWidths();

        if (cWidths.count()>=5) {
            m_view->setColumnWidth(Model::Layouts::ACTIVITYCOLUMN, cWidths[4].toInt());
        }
    } else {
        m_view->setColumnHidden(Model::Layouts::SHAREDCOLUMN, true);
    }
}

void Layouts::setOriginalInMultipleMode(bool inMultiple)
{
    o_originalInMultipleMode = inMultiple;
    setInMultipleMode(inMultiple);
}

int Layouts::rowForId(QString id) const
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowId = m_model->data(m_model->index(i, Model::Layouts::IDCOLUMN), Qt::DisplayRole).toString();

        if (rowId == id) {
            return i;
        }
    }

    return -1;
}

int Layouts::rowForName(QString layoutName) const
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowName = m_model->data(m_model->index(i, Model::Layouts::NAMECOLUMN), Qt::DisplayRole).toString();

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
    int pos_ = name.lastIndexOf(QRegExp(QString("[-][0-9]+")));

    if (m_model->containsCurrentName(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (m_model->containsCurrentName(name)) {
        name = namePart + "-" + QString::number(i);
        i++;
    }

    return name;
}

void Layouts::removeSelected()
{
    if (m_view->currentIndex().row() < 0) {
        return;
    }

    Data::Layout selected = selectedLayout();

    if (m_corona->layoutsManager()->synchronizer()->layout(selected.originalName())) {
        return;
    }

    int row = m_view->currentIndex().row();
    m_model->removeRow(row);

    row = qMin(row, m_model->rowCount() - 1);
    m_view->selectRow(row);
}

void Layouts::toggleLockedForSelected()
{
    if (m_view->currentIndex().row() < 0) {
        return;
    }

    Data::Layout selected = selectedLayout();

    m_model->setData(m_model->index(m_view->currentIndex().row(), Model::Layouts::NAMECOLUMN), !selected.isLocked, Settings::Model::Layouts::LAYOUTISLOCKEDROLE);
}

void Layouts::toggleSharedForSelected()
{
    if (m_view->currentIndex().row() < 0) {
        return;
    }

    Data::Layout selected = selectedLayout();

    if (selected.isShared()) {
        m_model->setData(m_model->index(m_view->currentIndex().row(), Model::Layouts::SHAREDCOLUMN), QStringList(), Qt::UserRole);
    } else {
      /*  bool assigned{false};
        QStringList assignedList;

        QStringList availableShares = availableSharesFor(row);

        for (const auto &id : availableShares) {
            QString name = nameForId(id);
            if (m_corona->layoutsManager()->synchronizer()->layout(name)) {
                assignedList << id;
                m_model->setData(m_model->index(row, Model::Layouts::SHAREDCOLUMN), assignedList, Qt::UserRole);
                assigned = true;
                break;
            }
        }

        if (!assigned && availableShares.count()>0) {
            assignedList << availableShares[0];
            m_model->setData(m_model->index(row, Model::Layouts::SHAREDCOLUMN), assignedList, Qt::UserRole);
            assigned = true;
        }*/
    }
}

QString Layouts::layoutNameForFreeActivities() const
{
    return m_model->layoutNameForFreeActivities();
}

void Layouts::setLayoutNameForFreeActivities(const QString &name, bool updateOriginalData)
{
    m_model->setLayoutNameForFreeActivities(name);

    if(updateOriginalData) {
        QString id = o_layoutsOriginalData.idForCurrentName(name);
        o_layoutsOriginalData.setLayoutForFreeActivities(id);
    }

    emit dataChanged();
}


void Layouts::loadLayouts()
{
    m_model->clear();

    //! The shares map needs to be constructed for start/scratch.
    //! We start feeding information with layout_names and during the process
    //! we update them to valid layout_ids
    Latte::Layouts::SharesMap sharesMap;

    int i = 0;
    QStringList brokenLayouts;

    if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
        m_corona->layoutsManager()->synchronizer()->syncActiveLayoutsToOriginalFiles();
    }

    Settings::Data::LayoutsTable layoutsBuffer;

    for (const auto layout : m_corona->layoutsManager()->layouts()) {
        Settings::Data::Layout original;
        original.id = QDir::homePath() + "/.config/latte/" + layout + ".layout.latte";

        CentralLayout *central = new CentralLayout(this, original.id);

        original.setOriginalName(central->name());
        original.background = central->background();
        original.color = central->color();
        original.textColor = central->textColor();
        original.isActive = (m_corona->layoutsManager()->synchronizer()->layout(original.originalName()) != nullptr);
        original.isLocked = !central->isWritable();
        original.isShownInMenu = central->showInMenu();
        original.hasDisabledBorders = central->disableBordersForMaximizedWindows();
        original.activities = central->activities();

        //! add central layout properties
        if (original.background.isEmpty()) {
            original.textColor = QString();
        }

        m_layouts[original.id] = central;

        //! create initial SHARES maps
        QString shared = central->sharedLayoutName();
        if (!shared.isEmpty()) {
            sharesMap[shared].append(original.id);
        }

        layoutsBuffer << original;

        qDebug() << "counter:" << i << " total:" << m_model->rowCount();

        i++;

        Latte::Layout::GenericLayout *generic = m_corona->layoutsManager()->synchronizer()->layout(central->name());

        if ((generic && generic->layoutIsBroken()) || (!generic && central->layoutIsBroken())) {
            brokenLayouts.append(central->name());
        }
    }

    //! update SHARES map keys in order to use the #settingsid(s)
    QStringList tempSharedNames;

    //! remove these records after updating
    for (QHash<const QString, QStringList>::iterator i=sharesMap.begin(); i!=sharesMap.end(); ++i) {
        tempSharedNames << i.key();
    }

    //! update keys
    for (QHash<const QString, QStringList>::iterator i=sharesMap.begin(); i!=sharesMap.end(); ++i) {
        QString shareid = layoutsBuffer.idForOriginalName(i.key());
        if (!shareid.isEmpty()) {
            sharesMap[shareid] = i.value();
        }
    }

    //! remove deprecated keys
    for (const auto &key : tempSharedNames) {
        sharesMap.remove(key);
    }

    qDebug() << "SHARES MAP ::: " << sharesMap;

    for (QHash<const QString, QStringList>::iterator i=sharesMap.begin(); i!=sharesMap.end(); ++i) {
        layoutsBuffer[i.key()].shares = i.value();
    }

    //! Send original loaded data to model
    m_model->setCurrentData(layoutsBuffer);
    m_model->setLayoutNameForFreeActivities(m_corona->universalSettings()->lastNonAssignedLayoutName());

    m_view->selectRow(rowForName(m_corona->layoutsManager()->currentLayoutName()));

    //! this line should be commented for debugging layouts window functionality
    m_view->setColumnHidden(Model::Layouts::IDCOLUMN, true);
    m_view->setColumnHidden(Model::Layouts::HIDDENTEXTCOLUMN, true);

    if (m_corona->universalSettings()->canDisableBorders()) {
        m_view->setColumnHidden(Model::Layouts::BORDERSCOLUMN, false);
    } else {
        m_view->setColumnHidden(Model::Layouts::BORDERSCOLUMN, true);
    }

    m_view->resizeColumnsToContents();

    QStringList columnWidths = m_corona->universalSettings()->layoutsColumnWidths();

    if (!columnWidths.isEmpty()) {
        for (int i=0; i<qMin(columnWidths.count(),4); ++i) {
            m_view->setColumnWidth(Model::Layouts::BACKGROUNDCOLUMN+i, columnWidths[i].toInt());
        }
    }

    o_layoutsOriginalData = m_model->currentData();

    //! there are broken layouts and the user must be informed!
    if (brokenLayouts.count() > 0) {
        auto msg = new QMessageBox(m_parent);
        msg->setIcon(QMessageBox::Warning);
        msg->setWindowTitle(i18n("Layout Warning"));
        msg->setText(i18n("The layout(s) <b>%0</b> have <i>broken configuration</i>!!! Please <b>remove them</b> to improve the system stability...").arg(brokenLayouts.join(",")));
        msg->setStandardButtons(QMessageBox::Ok);

        msg->open();
    }
}

void Layouts::addLayoutForFile(QString file, QString layoutName, bool newTempDirectory, bool showNotification)
{
    if (layoutName.isEmpty()) {
        layoutName = CentralLayout::layoutName(file);
    }

    layoutName = uniqueLayoutName(layoutName);

    Settings::Data::Layout copied;

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

    if (m_layouts.contains(copied.id)) {
        CentralLayout *oldSettings = m_layouts.take(copied.id);
        delete oldSettings;
    }

    CentralLayout *settings = new CentralLayout(this, copied.id);
    m_layouts[copied.id] = settings;

    copied.setOriginalName(uniqueLayoutName(layoutName));
    copied.color = settings->color();
    copied.textColor = settings->textColor();
    copied.background = settings->background();
    copied.isLocked = !settings->isWritable();
    copied.isShownInMenu = settings->showInMenu();
    copied.hasDisabledBorders = settings->disableBordersForMaximizedWindows();

    if (copied.background.isEmpty()) {
        copied.textColor = QString();
    }

    m_model->appendLayout(copied);

  //  ui->layoutsView->selectRow(row);

    if (showNotification) {
        //NOTE: The pointer is automatically deleted when the event is closed
        auto notification = new KNotification("import-done", KNotification::CloseOnTimeout);
        notification->setText(i18nc("import-done", "Layout: <b>%0</b> imported successfully<br>").arg(layoutName));
        notification->sendEvent();
    }
}

void Layouts::copySelectedLayout()
{
    int row = m_view->currentIndex().row();

    if (row < 0) {
        return;
    }

    Settings::Data::Layout selected = selectedLayout();

    //! Update original layout before copying if this layout is active
    if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
        Latte::Layout::GenericLayout *generic = m_corona->layoutsManager()->synchronizer()->layout(selected.originalName());
        if (generic) {
            generic->syncToLayoutFile();
        }
    }

    Settings::Data::Layout copied = selected;

    copied.setOriginalName(uniqueLayoutName(selected.currentName()));
    copied.id = uniqueTempDirectory() + "/" + copied.originalName() + ".layout.latte";;
    copied.activities = QStringList();
    copied.isLocked = false;

    QFile(selected.id).copy(copied.id);
    QFileInfo newFileInfo(copied.id);

    if (newFileInfo.exists() && !newFileInfo.isWritable()) {
        QFile(copied.id).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }

    CentralLayout *settings = new CentralLayout(this, copied.id);

    m_layouts[copied.id] = settings;
    m_model->appendLayout(copied);

    m_view->selectRow(row + 1);
}

void Layouts::importLayoutsFromV1ConfigFile(QString file)
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
            if (m_corona->layoutsManager()->importer()->importOldLayout(applets, name, false, tempDir.absolutePath())) {
                addLayoutForFile(tempDir.absolutePath() + "/" + name + ".layout.latte", name, false);
            }

            QString alternativeName = name + "-" + i18nc("layout", "Alternative");

            if (m_corona->layoutsManager()->importer()->importOldLayout(applets, alternativeName, false, tempDir.absolutePath())) {
                addLayoutForFile(tempDir.absolutePath() + "/" + alternativeName + ".layout.latte", alternativeName, false);
            }
        }
    }
}

void Layouts::reset()
{
    setOriginalInMultipleMode(o_originalInMultipleMode);
    m_model->setCurrentData(o_layoutsOriginalData);
    m_view->selectRow(rowForName(m_corona->layoutsManager()->currentLayoutName()));
}

void Layouts::save()
{
    //! Update Layouts
    QStringList knownActivities = m_corona->layoutsManager()->synchronizer()->activities();

    QTemporaryDir layoutTempDir;

    qDebug() << "Temporary Directory ::: " << layoutTempDir.path();

    QStringList fromRenamePaths;
    QStringList toRenamePaths;
    QStringList toRenameNames;

    QString switchToLayout;

    QHash<QString, Latte::Layout::GenericLayout *> activeLayoutsToRename;

    Settings::Data::LayoutsTable removedLayouts = o_layoutsOriginalData.subtracted(m_model->currentData());

    //! remove layouts that have been removed from the user
    for (int i=0; i<removedLayouts.rowCount(); ++i) {
        QFile(removedLayouts[i].id).remove();

        if (m_layouts.contains(removedLayouts[i].id)) {
            CentralLayout *removedLayout = m_layouts.take(removedLayouts[i].id);
            delete removedLayout;
        }
    }

    for (int i = 0; i < m_model->rowCount(); ++i) {
        Data::Layout iLayout = m_model->at(i);
        QStringList cleanedActivities;

        //!update only activities that are valid
        for (const auto &activity : iLayout.activities) {
            if (knownActivities.contains(activity) && activity != Settings::Data::Layout::FREEACTIVITIESID) {
                cleanedActivities.append(activity);
            }
        }

        //qDebug() << i << ". " << id << " - " << color << " - " << name << " - " << menu << " - " << lActivities;
        //! update the generic parts of the layouts
        bool isOriginalLayout = o_layoutsOriginalData.containsId(iLayout.id);
        Latte::Layout::GenericLayout *genericActive= isOriginalLayout ? m_corona->layoutsManager()->synchronizer()->layout(iLayout.originalName()) : nullptr;
        Latte::Layout::GenericLayout *generic = genericActive ? genericActive : m_layouts[iLayout.id];

        //! unlock read-only layout
        if (!generic->isWritable()) {
            generic->unlock();
        }

        if (iLayout.color.startsWith("/")) {
            //it is image file in such case
            if (iLayout.color != generic->background()) {
                generic->setBackground(iLayout.color);
            }

            if (generic->textColor() != iLayout.textColor) {
                generic->setTextColor(iLayout.textColor);
            }
        } else {
            if (iLayout.color != generic->color()) {
                generic->setColor(iLayout.color);
                generic->setBackground(QString());
                generic->setTextColor(QString());
            }
        }

        //! update only the Central-specific layout parts
        CentralLayout *centralActive = isOriginalLayout ? m_corona->layoutsManager()->synchronizer()->centralLayout(iLayout.originalName()) : nullptr;
        CentralLayout *central = centralActive ? centralActive : m_layouts[iLayout.id];

        if (central->showInMenu() != iLayout.isShownInMenu) {
            central->setShowInMenu(iLayout.isShownInMenu);
        }

        if (central->disableBordersForMaximizedWindows() != iLayout.hasDisabledBorders) {
            central->setDisableBordersForMaximizedWindows(iLayout.hasDisabledBorders);
        }

        if (central->activities() != cleanedActivities) {
            central->setActivities(cleanedActivities);
        }

        //! If the layout name changed OR the layout path is a temporary one
        if (iLayout.nameWasEdited()) {
            //! If the layout is Active in MultipleLayouts
            if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts && generic->isActive()) {
                qDebug() << " Active Layout Should Be Renamed From : " << generic->name() << " TO :: " << iLayout.currentName();
                activeLayoutsToRename[iLayout.currentName()] = generic;
            }

            QString tempFile = layoutTempDir.path() + "/" + QString(generic->name() + ".layout.latte");
            qDebug() << "new temp file ::: " << tempFile;

            if ((m_corona->layoutsManager()->memoryUsage() == Types::SingleLayout) && (generic->name() == m_corona->layoutsManager()->currentLayoutName())) {
                switchToLayout = iLayout.currentName();
            }

            generic = m_layouts.take(iLayout.id);
            delete generic;

            QFile(iLayout.id).rename(tempFile);

            fromRenamePaths.append(iLayout.id);
            toRenamePaths.append(tempFile);
            toRenameNames.append(iLayout.currentName());
        }
    }

    //! this is necessary in case two layouts have to swap names
    //! so we copy first the layouts in a temp directory and afterwards all
    //! together we move them in the official layout directory
    for (int i = 0; i < toRenamePaths.count(); ++i) {
        QString newFile = QDir::homePath() + "/.config/latte/" + toRenameNames[i] + ".layout.latte";
        QFile(toRenamePaths[i]).rename(newFile);

        CentralLayout *nLayout = new CentralLayout(this, newFile);
        m_layouts[newFile] = nLayout;

        //! updating the #SETTINGSID in the model for the layout that was renamed
        for (int j = 0; j < m_model->rowCount(); ++j) {
            Data::Layout jLayout = m_model->at(j);

            if (jLayout.id == fromRenamePaths[i]) {
                m_model->setData(m_model->index(j, Model::Layouts::IDCOLUMN), newFile, Qt::DisplayRole);
            }
        }
    }

    if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
        for (const auto &newLayoutName : activeLayoutsToRename.keys()) {
            Latte::Layout::GenericLayout *layoutPtr = activeLayoutsToRename[newLayoutName];
            qDebug() << " Active Layout of Type: " << layoutPtr->type() << " Is Renamed From : " << activeLayoutsToRename[newLayoutName]->name() << " TO :: " << newLayoutName;
            layoutPtr->renameLayout(newLayoutName);
        }
    }

    //! lock layouts in the end when the user has chosen it
    for (int i = 0; i < m_model->rowCount(); ++i) {
        Data::Layout layout = m_model->at(i);
        Latte::Layout::GenericLayout *layoutPtr = m_corona->layoutsManager()->synchronizer()->layout(layout.originalName());

        if (!layoutPtr && m_layouts.contains(layout.id)) {
            layoutPtr = m_layouts[layout.id];
        }

        if (layout.isLocked && layoutPtr && layoutPtr->isWritable()) {
            layoutPtr->lock();
        }
    }

    //! update SharedLayouts that are Active
    syncActiveShares();

    //! reload layouts in layoutsmanager
    m_corona->layoutsManager()->synchronizer()->loadLayouts();

    if (!m_model->layoutNameForFreeActivities().isEmpty()) {
        //! make sure that there is a layout for free activities
        //! send to layout manager in which layout to switch
        Latte::Types::LayoutsMemoryUsage inMemoryOption = Latte::Types::SingleLayout;

        if (inMultipleMode()) {
            inMemoryOption = Latte::Types::MultipleLayouts;
        }

        if (m_corona->layoutsManager()->memoryUsage() != inMemoryOption) {
            Types::LayoutsMemoryUsage previousMemoryUsage = m_corona->layoutsManager()->memoryUsage();
            m_corona->layoutsManager()->setMemoryUsage(inMemoryOption);

            m_corona->layoutsManager()->switchToLayout(m_model->layoutNameForFreeActivities(), previousMemoryUsage);
        } else {
            if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
                m_corona->layoutsManager()->synchronizer()->syncMultipleLayoutsToActivities(m_model->layoutNameForFreeActivities());
            } else {
                m_corona->layoutsManager()->switchToLayout(m_model->layoutNameForFreeActivities());
            }
        }
    }

    m_model->applyCurrentNames();

    o_layoutsOriginalData = m_model->currentData();
    o_originalInMultipleMode = m_model->inMultipleMode();

    emit dataChanged();
}

void Layouts::syncActiveShares()
{
    if (m_corona->layoutsManager()->memoryUsage() != Types::MultipleLayouts) {
        return;
    }

    Settings::Data::LayoutsTable currentLayoutsData = m_model->currentData();

    Latte::Layouts::SharesMap  currentSharesNamesMap = currentLayoutsData.sharesMap();
    QStringList originalSharesIds = o_layoutsOriginalData.allSharesIds();
    QStringList currentSharesIds = currentLayoutsData.allSharesIds();

    QStringList deprecatedSharesIds = Latte::subtracted(originalSharesIds, currentSharesIds);
    QStringList deprecatedSharesNames;

    for(int i=0; i<deprecatedSharesIds.count(); ++i) {
        QString shareId = deprecatedSharesIds[i];

        if (currentLayoutsData.containsId(shareId)) {
            deprecatedSharesNames << currentLayoutsData[shareId].currentName();
        } else if (o_layoutsOriginalData.containsId(shareId)) {
            deprecatedSharesNames << o_layoutsOriginalData[shareId].currentName();
        }
    }

    qDebug() << " CURRENT SHARES NAMES MAP  :: " << currentSharesNamesMap;
    qDebug() << " DEPRECATED SHARES ::";

    m_corona->layoutsManager()->synchronizer()->syncActiveShares(currentSharesNamesMap, deprecatedSharesNames);
}

void Layouts::saveColumnWidths()
{
    //! save column widths
    QStringList columnWidths;
    columnWidths << QString::number(m_view->columnWidth(Model::Layouts::BACKGROUNDCOLUMN));
    columnWidths << QString::number(m_view->columnWidth(Model::Layouts::NAMECOLUMN));
    columnWidths << QString::number(m_view->columnWidth(Model::Layouts::MENUCOLUMN));
    columnWidths << QString::number(m_view->columnWidth(Model::Layouts::BORDERSCOLUMN));

    if (inMultipleMode()) {
        columnWidths << QString::number(m_view->columnWidth(Model::Layouts::ACTIVITYCOLUMN));
    } else {
        //! In Single Mode, keed recorded value for ACTIVITYCOLUMN
        QStringList currentWidths = m_corona->universalSettings()->layoutsColumnWidths();
        if (currentWidths.count()>=5) {
            columnWidths << currentWidths[4];
        }
    }

    m_corona->universalSettings()->setLayoutsColumnWidths(columnWidths);
}

}
}
}
