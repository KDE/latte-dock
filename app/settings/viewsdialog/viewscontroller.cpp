/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewscontroller.h"

// local
#include <config-latte.h>
#include "ui_viewsdialog.h"
#include "viewsdialog.h"
#include "viewshandler.h"
#include "viewsmodel.h"
#include "viewstableview.h"
#include "delegates/namedelegate.h"
#include "delegates/singleoptiondelegate.h"
#include "delegates/singletextdelegate.h"
#include "../generic/generictools.h"
#include "../settingsdialog/templateskeeper.h"
#include "../../data/errorinformationdata.h"
#include "../../layout/genericlayout.h"
#include "../../layout/centrallayout.h"
#include "../../layouts/manager.h"
#include "../../layouts/synchronizer.h"
#include "../../view/view.h"

// Qt
#include <QHeaderView>
#include <QItemSelection>

// KDE
#include <KMessageWidget>
#include <KSharedConfig>
#include <KIO/OpenUrlJob>


namespace Latte {
namespace Settings {
namespace Controller {


Views::Views(Settings::Handler::ViewsHandler *parent)
    : QObject(parent),
      m_handler(parent),
      m_model(new Model::Views(this, m_handler->corona())),
      m_proxyModel(new QSortFilterProxyModel(this)),
      m_view(m_handler->ui()->viewsTable),
      m_storage(KConfigGroup(KSharedConfig::openConfig(),"LatteSettingsDialog").group("ViewsDialog"))
{
    loadConfig();
    m_proxyModel->setSourceModel(m_model);

    connect(m_model, &QAbstractItemModel::dataChanged, this, &Views::dataChanged);
    connect(m_model, &Model::Views::rowsInserted, this, &Views::dataChanged);
    connect(m_model, &Model::Views::rowsRemoved, this, &Views::dataChanged);

    connect(m_handler, &Handler::ViewsHandler::currentLayoutChanged, this, &Views::onCurrentLayoutChanged);

    init();
}

Views::~Views()
{
    saveConfig();
}

QAbstractItemModel *Views::proxyModel() const
{
    return m_proxyModel;
}

QAbstractItemModel *Views::baseModel() const
{
    return m_model;
}

QTableView *Views::view() const
{
    return m_view;
}

void Views::init()
{
    m_view->setModel(m_proxyModel);
    //m_view->setHorizontalHeader(m_headerView);
    m_view->verticalHeader()->setVisible(false);
    m_view->setSortingEnabled(true);

    m_proxyModel->setSortRole(Model::Views::SORTINGROLE);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_view->sortByColumn(m_viewSortColumn, m_viewSortOrder);

    m_view->setItemDelegateForColumn(Model::Views::IDCOLUMN, new Settings::View::Delegate::SingleText(this));
    m_view->setItemDelegateForColumn(Model::Views::NAMECOLUMN, new Settings::View::Delegate::NameDelegate(this));
    m_view->setItemDelegateForColumn(Model::Views::SCREENCOLUMN, new Settings::View::Delegate::SingleOption(this));
    m_view->setItemDelegateForColumn(Model::Views::EDGECOLUMN, new Settings::View::Delegate::SingleOption(this));
    m_view->setItemDelegateForColumn(Model::Views::ALIGNMENTCOLUMN, new Settings::View::Delegate::SingleOption(this));
    m_view->setItemDelegateForColumn(Model::Views::SUBCONTAINMENTSCOLUMN, new Settings::View::Delegate::SingleText(this));

    applyColumnWidths();

    m_cutAction = new QAction(QIcon::fromTheme("edit-cut"), i18n("Cut"), m_view);
    m_cutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));
    connect(m_cutAction, &QAction::triggered, this, &Views::cutSelectedViews);

    m_copyAction = new QAction(QIcon::fromTheme("edit-copy"), i18n("Copy"), m_view);
    m_copyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    connect(m_copyAction, &QAction::triggered, this, &Views::copySelectedViews);

    m_pasteAction = new QAction(QIcon::fromTheme("edit-paste"), i18n("Paste"), m_view);
    m_pasteAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
    connect(m_pasteAction, &QAction::triggered, this, &Views::pasteSelectedViews);

    m_duplicateAction = new QAction(QIcon::fromTheme("edit-copy"), i18n("Duplicate Here"), m_view);
    m_duplicateAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    connect(m_duplicateAction, &QAction::triggered, this, &Views::duplicateSelectedViews);

    m_view->addAction(m_cutAction);
    m_view->addAction(m_copyAction);
    m_view->addAction(m_duplicateAction);
    m_view->addAction(m_pasteAction);

    onSelectionsChanged();

    connect(m_view, &View::ViewsTableView::selectionsChanged, this, &Views::onSelectionsChanged);
    connect(m_view, &QObject::destroyed, this, &Views::storeColumnWidths);

    connect(m_view->horizontalHeader(), &QObject::destroyed, this, [&]() {
        m_viewSortColumn = m_view->horizontalHeader()->sortIndicatorSection();
        m_viewSortOrder = m_view->horizontalHeader()->sortIndicatorOrder();
    });
}

void Views::reset()
{
    m_model->resetData();

    //! Clear any templates keeper data in order to produce reupdates if needed
    m_handler->layoutsController()->templatesKeeper()->clear();
}

bool Views::hasChangedData() const
{
    return m_model->hasChangedData();
}

bool Views::hasSelectedView() const
{
    return m_view->selectionModel()->hasSelection();
}

int Views::selectedViewsCount() const
{
    return m_view->selectionModel()->selectedRows(Model::Views::IDCOLUMN).count();
}

int Views::rowForId(QString id) const
{
    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        QString rowId = m_proxyModel->data(m_proxyModel->index(i, Model::Views::IDCOLUMN), Qt::UserRole).toString();

        if (rowId == id) {
            return i;
        }
    }

    return -1;
}

const Data::ViewsTable Views::selectedViewsCurrentData() const
{
    Data::ViewsTable selectedviews;

    if (!hasSelectedView()) {
        return selectedviews;
    }

    QModelIndexList layoutidindexes = m_view->selectionModel()->selectedRows(Model::Views::IDCOLUMN);

    for(int i=0; i<layoutidindexes.count(); ++i) {
        QString selectedid = layoutidindexes[i].data(Qt::UserRole).toString();
        selectedviews <<  m_model->currentData(selectedid);
    }

    return selectedviews;
}

const Latte::Data::View Views::appendViewFromViewTemplate(const Data::View &view)
{
    Data::View newview = view;
    newview.name = uniqueViewName(view.name);
    m_model->appendTemporaryView(newview);
    return newview;
}

const Latte::Data::View Views::currentData(const QString &id)
{
    return m_model->currentData(id);
}

Data::ViewsTable Views::selectedViewsForClipboard()
{
    Data::ViewsTable clipboardviews;
    if (!hasSelectedView()) {
        return clipboardviews;
    }

    Data::ViewsTable selectedviews = selectedViewsCurrentData();
    Latte::Data::Layout currentlayout = m_handler->currentData();

    for(int i=0; i<selectedviews.rowCount(); ++i) {
        if (selectedviews[i].state() == Data::View::IsInvalid) {
            continue;
        }

        Latte::Data::View copiedview = selectedviews[i];

        if (selectedviews[i].state() == Data::View::IsCreated) {
            QString storedviewpath = m_handler->layoutsController()->templatesKeeper()->storedView(currentlayout.id, selectedviews[i].id);
            copiedview.setState(Data::View::OriginFromLayout, storedviewpath, currentlayout.id, selectedviews[i].id);
        } else if (selectedviews[i].state() == Data::View::OriginFromViewTemplate) {
            copiedview.setState(Data::View::OriginFromViewTemplate, selectedviews[i].originFile(), currentlayout.id, selectedviews[i].id);
        } else if (selectedviews[i].state() == Data::View::OriginFromLayout) {
            //! is already in valid values
        }

        copiedview.isActive = false;
        clipboardviews << copiedview;

    }

    return clipboardviews;
}

void Views::copySelectedViews()
{
    qDebug() << Q_FUNC_INFO;

    if (!hasSelectedView()) {
        return;
    }

    //! reset cut substates for views
    Data::ViewsTable currentviews = m_model->currentViewsData();
    for (int i=0; i<currentviews.rowCount(); ++i) {
        Data::View cview = currentviews[i];
        cview.isMoveOrigin = false;
        m_model->updateCurrentView(cview.id, cview);
    }

    Data::ViewsTable clipboardviews = selectedViewsForClipboard();

    //! reset cut substates for views
    for (int i=0; i<clipboardviews.rowCount(); ++i) {
        clipboardviews[i].isMoveOrigin = false;

        /*   Data::View tempview = m_model->currentData(clipboardviews[i].id);
        tempview.isMoveOrigin = false;
        m_model->updateCurrentView(tempview.id, tempview);*/
    }

    m_handler->layoutsController()->templatesKeeper()->setClipboardContents(clipboardviews);
}

void Views::cutSelectedViews()
{
    qDebug() << Q_FUNC_INFO;

    if (!hasSelectedView()) {
        return;
    }

    //! reset previous move records
    Data::ViewsTable currentviews = m_model->currentViewsData();
    for (int i=0; i<currentviews.rowCount(); ++i) {
        Data::View cview = currentviews[i];
        cview.isMoveOrigin = false;
        m_model->updateCurrentView(cview.id, cview);
    }

    Data::ViewsTable clipboardviews = selectedViewsForClipboard();

    //! activate cut substates for views
    for (int i=0; i<clipboardviews.rowCount(); ++i) {
        clipboardviews[i].isMoveOrigin = true;

        Data::View tempview = m_model->currentData(clipboardviews[i].id);
        tempview.isMoveOrigin = true;
        m_model->updateCurrentView(tempview.id, tempview);
    }

    m_handler->layoutsController()->templatesKeeper()->setClipboardContents(clipboardviews);
}

void Views::pasteSelectedViews()
{
    Data::ViewsTable clipboardviews = m_handler->layoutsController()->templatesKeeper()->clipboardContents();
    Latte::Data::Layout currentlayout = m_handler->currentData();

    bool hascurrentlayoutcuttedviews{false};

    for(int i=0; i<clipboardviews.rowCount(); ++i) {
        if (clipboardviews[i].isMoveOrigin && clipboardviews[i].originLayout() == currentlayout.id) {
            hascurrentlayoutcuttedviews = true;
            continue;
        }

        if (clipboardviews[i].isMoveOrigin) {
            //! update cut flags only for real cutted view and not for copied one
            clipboardviews[i].isMoveOrigin = false;
            clipboardviews[i].isMoveDestination = true;
        }

        appendViewFromViewTemplate(clipboardviews[i]);
    }

    if (hascurrentlayoutcuttedviews) {
        m_handler->showInlineMessage(i18n("Docks and panels from <b>Paste</b> action are already present in current layout"),
                                     KMessageWidget::Warning);
    }
}

void Views::duplicateSelectedViews()
{
    qDebug() << Q_FUNC_INFO;

    if (!hasSelectedView()) {
        return;
    }

    Data::ViewsTable selectedviews = selectedViewsCurrentData();
    Latte::Data::Layout currentlayout = m_handler->currentData();

    for(int i=0; i<selectedviews.rowCount(); ++i) {
        if (selectedviews[i].state() == Data::View::IsCreated) {
            QString storedviewpath = m_handler->layoutsController()->templatesKeeper()->storedView(currentlayout.id, selectedviews[i].id);
            Latte::Data::View duplicatedview = selectedviews[i];
            duplicatedview.setState(Data::View::OriginFromLayout, storedviewpath, currentlayout.id, selectedviews[i].id);
            duplicatedview.isActive = false;
            appendViewFromViewTemplate(duplicatedview);
        } else if (selectedviews[i].state() == Data::View::OriginFromViewTemplate
                   || selectedviews[i].state() == Data::View::OriginFromLayout) {
            Latte::Data::View duplicatedview = selectedviews[i];
            duplicatedview.isActive = false;
            appendViewFromViewTemplate(duplicatedview);
        }
    }
}

void Views::removeSelectedViews()
{
    if (!hasSelectedView()) {
        return;
    }

    Data::ViewsTable selectedviews = selectedViewsCurrentData();;

    int selectionheadrow = m_model->rowForId(selectedviews[0].id);

    for (int i=0; i<selectedviews.rowCount(); ++i) {
        m_model->removeView(selectedviews[i].id);
    }

    m_view->selectRow(qBound(0, selectionheadrow, m_model->rowCount()-1));
}

void Views::selectRow(const QString &id)
{
    m_view->selectRow(rowForId(id));
}

void Views::onCurrentLayoutChanged()
{   
    Data::Layout currentlayoutdata = m_handler->currentData();

    Data::ViewsTable clipboardviews = m_handler->layoutsController()->templatesKeeper()->clipboardContents();

    if (!clipboardviews.isEmpty()) {
        //! clipboarded views needs to update the relevant flags to loaded views
        for (int i=0; i<currentlayoutdata.views.rowCount(); ++i) {
            QString vid = currentlayoutdata.views[i].id;

            if (!clipboardviews.containsId(vid)) {
                continue;
            }

            if (clipboardviews[vid].isMoveOrigin && (clipboardviews[vid].originLayout() == currentlayoutdata.id)) {
                currentlayoutdata.views[vid].isMoveOrigin = true;
            }
        }
    }

    m_model->setOriginalData(currentlayoutdata.views);

    //! track viewscountchanged signal for current active layout scenario
    for (const auto &var : m_currentLayoutConnections) {
        QObject::disconnect(var);
    }

    Latte::CentralLayout *currentlayout = m_handler->layoutsController()->centralLayout(currentlayoutdata.id);

    if (currentlayout && currentlayout->isActive()) {
        m_currentLayoutConnections << connect(currentlayout, &Layout::GenericLayout::viewsCountChanged, this, [&, currentlayout](){
            m_model->updateActiveStatesBasedOn(currentlayout);
        });
    }

    messagesForErrorsWarnings(currentlayout);
}

void Views::onSelectionsChanged()
{
    bool hasselectedview = hasSelectedView();

    m_cutAction->setVisible(hasselectedview);
    m_copyAction->setVisible(hasselectedview);
    m_duplicateAction->setVisible(hasselectedview);
    m_pasteAction->setEnabled(m_handler->layoutsController()->templatesKeeper()->hasClipboardContents());
}

int Views::viewsForRemovalCount() const
{
    if (!hasChangedData()) {
        return 0;
    }

    Latte::Data::ViewsTable originalViews = m_model->originalViewsData();
    Latte::Data::ViewsTable currentViews = m_model->currentViewsData();
    Latte::Data::ViewsTable removedViews = originalViews.subtracted(currentViews);

    return removedViews.rowCount();
}

bool Views::hasValidOriginView(const Data::View &view)
{
    bool viewidisinteger{true};
    int vid_int = view.originView().toInt(&viewidisinteger);
    QString vid_str = view.originView();

    if (vid_str.isEmpty() || !viewidisinteger || vid_int<=0) {
        return false;
    }

    return true;
}

CentralLayout *Views::originLayout(const Data::View &view)
{
    QString origincurrentid = view.originLayout();
    Data::Layout originlayoutdata = m_handler->layoutsController()->originalData(origincurrentid);

    Latte::CentralLayout *originactive = m_handler->layoutsController()->isLayoutOriginal(origincurrentid) ?
                m_handler->corona()->layoutsManager()->synchronizer()->centralLayout(originlayoutdata.name) : nullptr;

    return originactive;
}

void Views::updateDoubledMoveDestinationRows() {
    //! only one isMoveDestination should exist for each unique move isMoveOrigin case
    //! all the rest that have been created through Cut/Paste or Duplicate options should become
    //! simple OriginFromViewTemplate cases

    for (int i=0; i<m_model->rowCount(); ++i) {
        Data::View baseview = m_model->at(i);

        if (!baseview.isMoveDestination || baseview.state()!=Data::View::OriginFromLayout) {
            continue;
        }

        for (int j=i+1; j<m_model->rowCount(); ++j) {
            Data::View subsequentview = m_model->at(j);

            if (subsequentview.isMoveDestination
                    && subsequentview.state() == Data::View::OriginFromLayout
                    && subsequentview.originFile() == baseview.originFile()
                    && subsequentview.originLayout() == baseview.originLayout()
                    && subsequentview.originView() == baseview.originView()) {
                //! this is a subsequent view that needs to be updated properly
                subsequentview.isMoveDestination = false;
                subsequentview.isMoveOrigin = false;
                subsequentview.setState(Data::View::OriginFromViewTemplate, subsequentview.originFile(), QString(), QString());
                m_model->updateCurrentView(subsequentview.id, subsequentview);
            }
        }
    }
}

void Views::messagesForErrorsWarnings(const Latte::CentralLayout *centralLayout, const bool &showNoErrorsMessage)
{
    if (!centralLayout) {
        return;
    }

    Data::Layout currentdata = centralLayout->data();

    m_model->clearErrorsAndWarnings();

    //! warnings
    if (currentdata.warnings > 0) {
        Data::WarningsList warnings = centralLayout->warnings();

        // show warnings
        for (int i=0; i< warnings.count(); ++i) {
            if (warnings[i].id == Data::Warning::ORPHANEDSUBCONTAINMENT) {
                messageForWarningOrphanedSubContainments(warnings[i]);
            } else if (warnings[i].id == Data::Warning::APPLETANDCONTAINMENTWITHSAMEID) {
                messageForWarningAppletAndContainmentWithSameId(warnings[i]);
            }
        }

        // count warnings per view
        for (int i=0; i<warnings.count(); ++i) {
            for (int j=0; j<warnings[i].information.rowCount(); ++j) {
                if (!warnings[i].information[j].containment.isValid()) {
                    continue;
                }

                QString cid = warnings[i].information[j].containment.storageId;
                Data::View view = m_model->currentData(cid);
                if (!view.isValid()) {
                    //! one step back from subcontainment to view in order to find the influenced view id
                    cid = m_model->viewForSubContainment(cid);
                    view = m_model->currentData(cid);
                }

                if (view.isValid()) {
                    view.warnings++;
                    m_model->updateCurrentView(cid, view);
                }
            }
        }
    }

    //! errors
    if (currentdata.errors > 0) {
        Data::ErrorsList errors = centralLayout->errors();

        // show errors
        for (int i=0; i< errors.count(); ++i) {
            if (errors[i].id == Data::Error::APPLETSWITHSAMEID) {
                messageForErrorAppletsWithSameId(errors[i]);
            } else if (errors[i].id == Data::Error::ORPHANEDPARENTAPPLETOFSUBCONTAINMENT) {
                messageForErrorOrphanedParentAppletOfSubContainment(errors[i]);
            }
        }

        // count errors per view
        for (int i=0; i<errors.count(); ++i) {
            for (int j=0; j<errors[i].information.rowCount(); ++j) {
                if (!errors[i].information[j].containment.isValid()) {
                    continue;
                }

                QString cid = errors[i].information[j].containment.storageId;
                Data::View view = m_model->currentData(cid);
                if (!view.isValid()) {
                    //! one step back from subcontainment to view in order to find the influenced view id
                    cid = m_model->viewForSubContainment(cid);
                    view = m_model->currentData(cid);
                }

                if (view.isValid()) {
                    view.errors++;
                    m_model->updateCurrentView(cid, view);
                }
            }
        }
    }

    m_handler->layoutsController()->setLayoutCurrentErrorsWarnings(currentdata.id, currentdata.errors, currentdata.warnings);

    if (showNoErrorsMessage && currentdata.errors == 0 && currentdata.warnings == 0) {
        m_handler->showInlineMessage(i18n("Really nice! You are good to go, your layout does not report any errors or warnings."),
                                     KMessageWidget::Positive,
                                     false);
    }

}

void Views::showDefaultPersistentErrorWarningInlineMessage(const QString &messageText,
                                                           const KMessageWidget::MessageType &messageType,
                                                           QList<QAction *> extraActions,
                                                           const bool &showOpenLayoutAction)
{
    QList<QAction *> actions;
    actions << extraActions;

    if (showOpenLayoutAction) {
        Data::Layout currentlayout = m_handler->currentData();

        //! add default action to open layout
        QAction *openlayoutaction = new QAction(i18n("Edit Layout"), this);
        openlayoutaction->setEnabled(!currentlayout.isActive);
        openlayoutaction->setIcon(QIcon::fromTheme("document-edit"));
        openlayoutaction->setData(currentlayout.id);
        actions << openlayoutaction;

        connect(openlayoutaction, &QAction::triggered, this, [&, openlayoutaction]() {
            QString file = openlayoutaction->data().toString();

            if (!file.isEmpty()) {
                auto job = new KIO::OpenUrlJob(QUrl::fromLocalFile(file), QStringLiteral("text/plain"), this);
                job->start();
                showDefaultInlineMessageValidator();
            }
        });
    }

    //! show message
    m_handler->showInlineMessage(messageText,
                                 messageType,
                                 true,
                                 actions);
}

void Views::showDefaultInlineMessageValidator()
{
    Data::Layout currentlayout = m_handler->currentData();

    //! add default action to open layout
    QAction *validateaction = new QAction(i18n("Validate"), this);
    validateaction->setIcon(QIcon::fromTheme("view-refresh"));
    validateaction->setData(currentlayout.id);

    QList<QAction *> actions;
    actions << validateaction;

    connect(validateaction, &QAction::triggered, this, [&, currentlayout]() {

        auto centrallayout = m_handler->layoutsController()->centralLayout(currentlayout.id);
        if (centrallayout && !centrallayout->isActive()) {
            KSharedConfigPtr lFile = KSharedConfig::openConfig(centrallayout->file());
            //! update configuration with latest changes
            lFile->reparseConfiguration();
        }

        messagesForErrorsWarnings(centrallayout, true);
    });

    QString messagetext = i18n("After you have made your layout file changes, please click <b>Validate</b> to confirm them.");

    //! show message
    m_handler->showInlineMessage(messagetext,
                                 KMessageWidget::Warning,
                                 true,
                                 actions);
}

void Views::messageForErrorAppletsWithSameId(const Data::Error &error)
{
    if (error.id != Data::Error::APPLETSWITHSAMEID) {
        return;
    }

    //! construct message
    QString message = i18nc("error id and title", "<b>Error #%1: %2</b> <br/>",error.id, error.name);
    message += "<br/>";
    message += i18n("In your layout there are two or more applets with same id. Such situation can create crashes, abnormal behavior and data loss when you activate and use this layout.<br/>");

    message += "<br/>";
    message += i18n("<b>Applets:</b><br/>");
    for (int i=0; i<error.information.rowCount(); ++i) {
        QString appletname = error.information[i].applet.visibleName();
        QString appletstorageid = error.information[i].applet.storageId;
        QString viewname = visibleViewName(error.information[i].containment.storageId);
        QString containmentname = viewname.isEmpty() ? error.information[i].containment.visibleName() : viewname;
        QString containmentstorageid = error.information[i].containment.storageId;
        message += i18nc("applets with same id error, applet name, applet id, containment name, containment id",
                         "&nbsp;&nbsp;• <b>%1</b> [#%2] inside  <b>%3</b> [#%4]<br/>",
                         appletname,
                         appletstorageid,
                         containmentname,
                         containmentstorageid);
    }

    message += "<br/>";
    message += i18n("<b>Possible Solutions:</b><br/>");
    message += i18n("&nbsp;&nbsp;1. Activate this layout and restart Latte<br/>");
    message += i18n("&nbsp;&nbsp;2. Remove the mentioned applets from your layout<br/>");
    message += i18n("&nbsp;&nbsp;3. Update manually the applets id when the layout is <b>not active</b><br/>");
    message += i18n("&nbsp;&nbsp;4. Remove this layout totally<br/>");

    showDefaultPersistentErrorWarningInlineMessage(message, KMessageWidget::Error);
}

void Views::messageForErrorOrphanedParentAppletOfSubContainment(const Data::Error &error)
{
    if (error.id != Data::Error::ORPHANEDPARENTAPPLETOFSUBCONTAINMENT) {
        return;
    }

    //! construct message
    QString message = i18nc("error id and title", "<b>Error #%1: %2</b> <br/><br/>", error.id, error.name);
    message += i18n("In your layout there are orphaned pseudo applets that link to unexistent subcontainments. Such case is for example a systemtray that has lost connection with its child applets. Such situation can create crashes, abnormal behavior and data loss when you activate and use this layout.<br/>");

    message += "<br/>";
    message += i18n("<b>Pseudo Applets:</b><br/>");
    for (int i=0; i<error.information.rowCount(); ++i) {
        if (!error.information[i].applet.isValid()) {
            continue;
        }

        QString appletname = error.information[i].applet.visibleName();
        QString appletstorageid = error.information[i].applet.storageId;
        QString viewname = visibleViewName(error.information[i].containment.storageId);
        QString containmentname = viewname.isEmpty() ? error.information[i].containment.visibleName() : viewname;
        QString containmentstorageid = error.information[i].containment.storageId;
        message += i18nc("orphaned pseudo applets, applet name, applet id, containment name, containment id",
                         "&nbsp;&nbsp;• <b>%1</b> [#%2] inside  <b>%3</b> [#%4]<br/>",
                         appletname,
                         appletstorageid,
                         containmentname,
                         containmentstorageid);
    }

    message += "<br/>";
    message += i18n("<b>Orphaned Subcontainments:</b><br/>");
    for (int i=0; i<error.information.rowCount(); ++i) {
        if (error.information[i].applet.isValid()) {
            continue;
        }

        QString viewname = visibleViewName(error.information[i].containment.storageId);
        QString containmentname = viewname.isEmpty() ? error.information[i].containment.visibleName() : viewname;
        QString containmentstorageid = error.information[i].containment.storageId;
        message += i18nc("orphaned subcontainments, containment name, containment id",
                         "&nbsp;&nbsp;• <b>%1</b> [#%2] <br/>",
                         containmentname,
                         containmentstorageid);
    }

    message += "<br/>";
    message += i18n("<b>Possible Solutions:</b><br/>");
    message += i18n("&nbsp;&nbsp;1. Update manually the subcontainment id inside pseudo applet settings when the layout is <b>not active</b><br/>");
    message += i18n("&nbsp;&nbsp;2. Remove this layout totally<br/>");

    //! show message
    showDefaultPersistentErrorWarningInlineMessage(message, KMessageWidget::Error);
}

void Views::messageForWarningAppletAndContainmentWithSameId(const Data::Warning &warning)
{
    if (warning.id != Data::Warning::APPLETANDCONTAINMENTWITHSAMEID) {
        return;
    }

    //! construct message
    QString message = i18nc("warning id and title", "<b>Warning #%1: %2</b> <br/><br/>", warning.id, warning.name);
    message += i18n("In your layout there are applets and containments with the same id. Such situation is not dangerous but it should not occur.<br/>");

    message += "<br/>";
    message += i18n("<b>Applets:</b><br/>");
    for (int i=0; i<warning.information.rowCount(); ++i) {
        if (!warning.information[i].applet.isValid()) {
            continue;
        }

        QString appletname = warning.information[i].applet.visibleName();
        QString appletstorageid = warning.information[i].applet.storageId;
        QString viewname = visibleViewName(warning.information[i].containment.storageId);
        QString containmentname = viewname.isEmpty() ? warning.information[i].containment.visibleName() : viewname;
        QString containmentstorageid = warning.information[i].containment.storageId;
        message += i18nc("applets, applet name, applet id, containment name, containment id",
                         "&nbsp;&nbsp;• <b>%1</b> [#%2] inside  <b>%3</b> [#%4]<br/>",
                         appletname,
                         appletstorageid,
                         containmentname,
                         containmentstorageid);
    }

    message += "<br/>";
    message += i18n("<b>Containments:</b><br/>");
    for (int i=0; i<warning.information.rowCount(); ++i) {
        if (warning.information[i].applet.isValid()) {
            continue;
        }

        QString viewname = visibleViewName(warning.information[i].containment.storageId);
        QString containmentname = viewname.isEmpty() ? warning.information[i].containment.visibleName() : viewname;
        QString containmentstorageid = warning.information[i].containment.storageId;
        message += i18nc("containments, containment name, containment id",
                         "&nbsp;&nbsp;• <b>%1</b> [#%2] <br/>",
                         containmentname,
                         containmentstorageid);
    }

    message += "<br/>";
    message += i18n("<b>Possible Solutions:</b><br/>");
    message += i18n("&nbsp;&nbsp;1. Update manually the containments or applets id when the layout is <b>not active</b><br/>");
    message += i18n("&nbsp;&nbsp;2. Remove any of the containments or applets that conflict with each other<br/>");

    //! show message
    showDefaultPersistentErrorWarningInlineMessage(message, KMessageWidget::Warning);
}

void Views::messageForWarningOrphanedSubContainments(const Data::Warning &warning)
{
    if (warning.id != Data::Warning::ORPHANEDSUBCONTAINMENT) {
        return;
    }

    QList<int> orphaned;

    //! construct message
    QString message = i18nc("warning id and title", "<b>Warning #%1: %2</b> <br/><br/>", warning.id, warning.name);
    message += i18n("In your layout there are orphaned subcontainments that are not used by any dock or panel. Such situation is not dangerous but it is advised to remove them in order to reduce memory usage.<br/>");

    message += "<br/>";
    message += i18n("<b>Orphaned Subcontainments:</b><br/>");
    for (int i=0; i<warning.information.rowCount(); ++i) {
        if (warning.information[i].applet.isValid()) {
            continue;
        }

        QString viewname = visibleViewName(warning.information[i].containment.storageId);
        QString containmentname = viewname.isEmpty() ? warning.information[i].containment.visibleName() : viewname;
        QString containmentstorageid = warning.information[i].containment.storageId;
        message += i18nc("orphaned subcontainments, containment name, containment id",
                         "&nbsp;&nbsp;• <b>%1</b> [#%2] <br/>",
                         containmentname,
                         containmentstorageid);

        orphaned << warning.information[i].containment.storageId.toInt();
    }

    message += "<br/>";
    message += i18n("<b>Possible Solutions:</b><br/>");
    message += i18n("&nbsp;&nbsp;1. Click <b>Repair</b> button in order to remove orphaned subcontainments<br/>");
    message += i18n("&nbsp;&nbsp;2. Remove manually orphaned subcontainments when the layout is <b>not active</b><br/>");

    //! add extra repair action
    QAction *repairlayoutaction = new QAction(i18n("Repair"), this);
    repairlayoutaction->setIcon(QIcon::fromTheme("dialog-yes"));
    QList<QAction *> extraactions;
    extraactions << repairlayoutaction;

    Latte::Data::Layout currentlayout = m_handler->currentData();

    connect(repairlayoutaction, &QAction::triggered, this, [&, currentlayout, orphaned]() {
        auto centrallayout = m_handler->layoutsController()->centralLayout(currentlayout.id);

        for (int i=0; i<orphaned.count(); ++i) {
            centrallayout->removeOrphanedSubContainment(orphaned[i]);
        }

        messagesForErrorsWarnings(centrallayout, true);
    });

    //! show message
    showDefaultPersistentErrorWarningInlineMessage(message,
                                                   KMessageWidget::Warning,
                                                   extraactions);
}

void Views::save()
{
    //! when this function is called we consider that removal has already been approved
    updateDoubledMoveDestinationRows();

    Latte::Data::Layout originallayout = m_handler->originalData();
    Latte::Data::Layout currentlayout = m_handler->currentData();
    Latte::CentralLayout *central = m_handler->layoutsController()->centralLayout(currentlayout.id);

    //! views in model
    Latte::Data::ViewsTable originalViews = m_model->originalViewsData();
    Latte::Data::ViewsTable currentViews = m_model->currentViewsData();
    Latte::Data::ViewsTable alteredViews = m_model->alteredViews();
    Latte::Data::ViewsTable newViews = m_model->newViews();

    QHash<QString, Data::View> newviewsresponses;
    QHash<QString, Data::View> cuttedpastedviews;
    QHash<QString, Data::View> cuttedpastedactiveviews;

    m_debugSaveCall++;
    qDebug() << "org.kde.latte ViewsDialog::save() call: " << m_debugSaveCall << "-------- ";

    //! add new views that are accepted
    for(int i=0; i<newViews.rowCount(); ++i){
        if (newViews[i].isMoveDestination) {
            CentralLayout *originActive = originLayout(newViews[i]);
            bool inmovebetweenactivelayouts = central->isActive() && originActive && central != originActive && hasValidOriginView(newViews[i]);

            if (inmovebetweenactivelayouts) {
                cuttedpastedactiveviews[newViews[i].id] = newViews[i];
                continue;
            }

            cuttedpastedviews[newViews[i].id] = newViews[i];
        }

        if (newViews[i].state() == Data::View::OriginFromViewTemplate) {
            Data::View addedview = central->newView(newViews[i]);

            newviewsresponses[newViews[i].id] = addedview;
        } else if (newViews[i].state() == Data::View::OriginFromLayout) {
            Data::View adjustedview = newViews[i];
            adjustedview.setState(Data::View::OriginFromViewTemplate, newViews[i].originFile(), QString(), QString());
            Data::View addedview = central->newView(adjustedview);

            newviewsresponses[newViews[i].id] = addedview;
        }
    }

    //! update altered views
    for (int i=0; i<alteredViews.rowCount(); ++i) {
        if (alteredViews[i].state() == Data::View::IsCreated && !alteredViews[i].isMoveOrigin) {
            qDebug() << "org.kde.latte ViewsDialog::save() updating altered view :: " << alteredViews[i];
            central->updateView(alteredViews[i]);
        }
    }

    //! remove deprecated views that have been removed from user
    Latte::Data::ViewsTable removedViews = originalViews.subtracted(currentViews);

    for (int i=0; i<removedViews.rowCount(); ++i) {
        qDebug() << "org.kde.latte ViewsDialog::save() real removing view :: " << removedViews[i];
        central->removeView(removedViews[i]);
    }

    //! remove deprecated views from external layouts that must be removed because of Cut->Paste Action
    for(const auto vid: cuttedpastedviews.keys()){
        bool viewidisinteger{true};
        int vid_int = cuttedpastedviews[vid].originView().toInt(&viewidisinteger);
        QString vid_str = cuttedpastedviews[vid].originView();

        if (vid_str.isEmpty() || !viewidisinteger || vid_int<=0) {
            //! ignore origin views that have not been created already
            continue;
        }

        qDebug() << "org.kde.latte ViewsDialog::save() removing cut-pasted view :: " << cuttedpastedviews[vid];

        //! Be Careful: Remove deprecated views from Cut->Paste Action
        QString origincurrentid = cuttedpastedviews[vid].originLayout();
        Data::Layout originlayout = m_handler->layoutsController()->originalData(origincurrentid);
        Latte::CentralLayout *origin = m_handler->layoutsController()->centralLayout(originlayout.id);

        Data::ViewsTable originviews = Latte::Layouts::Storage::self()->views(origin);

        if (originviews.containsId(vid_str)) {
            origin->removeView(originviews[vid_str]);
        }
    }

    //! move active views between different active layouts
    for (const auto vid: cuttedpastedactiveviews.keys()) {
        Data::View pastedactiveview = cuttedpastedactiveviews[vid];
        uint originviewid = pastedactiveview.originView().toUInt();
        CentralLayout *origin = originLayout(pastedactiveview);
        QString originlayoutname = origin->name();
        QString destinationlayoutname = originallayout.name;

        auto view = origin->viewForContainment(originviewid);

        QString tempviewid = pastedactiveview.id;
        pastedactiveview.id = QString::number(originviewid);

        qDebug() << "org.kde.latte ViewsDialog::save() move to another layout cutted-pasted active view :: " << pastedactiveview;

        if (view) {
            //! onscreen_view->onscreen_view
            //! onscreen_view->offscreen_view
            pastedactiveview.setState(pastedactiveview.state(), pastedactiveview.originFile(), destinationlayoutname, pastedactiveview.originView());
            origin->updateView(pastedactiveview);
        } else {
            //! offscreen_view->onscreen_view
            m_handler->corona()->layoutsManager()->moveView(originlayoutname, originviewid, destinationlayoutname);
            //!is needed in order for layout to not trigger another move
            pastedactiveview.setState(Data::View::IsCreated, QString(), QString(), QString());
            central->updateView(pastedactiveview);
        }

        pastedactiveview.setState(Data::View::IsCreated, QString(), QString(), QString());
        newviewsresponses[tempviewid] = pastedactiveview;
    }

    //! update
    if ((removedViews.rowCount() > 0) || (newViews.rowCount() > 0)) {
        m_handler->corona()->layoutsManager()->synchronizer()->syncActiveLayoutsToOriginalFiles();
    }

    //! update model for newly added views
    for (const auto vid: newviewsresponses.keys()) {
        m_model->setOriginalView(vid, newviewsresponses[vid]);
    }

    //! update all table with latest data and make the original one
    currentViews = m_model->currentViewsData();
    m_model->setOriginalData(currentViews);

    //! update model activeness
    if (central->isActive()) {
        m_model->updateActiveStatesBasedOn(central);
    }

    //! Clear any templates keeper data in order to produce reupdates if needed
    m_handler->layoutsController()->templatesKeeper()->clear();
}

QString Views::uniqueViewName(QString name)
{
    if (name.isEmpty()) {
        return name;
    }

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

QString Views::visibleViewName(const QString &id) const
{
    if (id.isEmpty()) {
        return QString();
    }

    Data::View view = m_model->currentData(id);

    if (view.isValid()) {
        return view.name;
    }

    return QString();

}

void Views::applyColumnWidths()
{
    m_view->horizontalHeader()->setSectionResizeMode(Model::Views::SUBCONTAINMENTSCOLUMN, QHeaderView::Stretch);

    if (m_viewColumnWidths.count()<(Model::Views::columnCount()-1)) {
        return;
    }

    m_view->setColumnWidth(Model::Views::IDCOLUMN, m_viewColumnWidths[0].toInt());
    m_view->setColumnWidth(Model::Views::NAMECOLUMN, m_viewColumnWidths[1].toInt());
    m_view->setColumnWidth(Model::Views::SCREENCOLUMN, m_viewColumnWidths[2].toInt());
    m_view->setColumnWidth(Model::Views::EDGECOLUMN, m_viewColumnWidths[3].toInt());
    m_view->setColumnWidth(Model::Views::ALIGNMENTCOLUMN, m_viewColumnWidths[4].toInt());
}

void Views::storeColumnWidths()
{
    if (m_viewColumnWidths.isEmpty() || (m_viewColumnWidths.count()<Model::Views::columnCount()-1)) {
        m_viewColumnWidths.clear();
        for (int i=0; i<Model::Views::columnCount(); ++i) {
            m_viewColumnWidths << "";
        }
    }

    m_viewColumnWidths[0] = QString::number(m_view->columnWidth(Model::Views::IDCOLUMN));
    m_viewColumnWidths[1] = QString::number(m_view->columnWidth(Model::Views::NAMECOLUMN));
    m_viewColumnWidths[2] = QString::number(m_view->columnWidth(Model::Views::SCREENCOLUMN));
    m_viewColumnWidths[3] = QString::number(m_view->columnWidth(Model::Views::EDGECOLUMN));
    m_viewColumnWidths[4] = QString::number(m_view->columnWidth(Model::Views::ALIGNMENTCOLUMN));
}

void Views::loadConfig()
{
    QStringList defaultcolumnwidths;
    defaultcolumnwidths << QString::number(59) << QString::number(256) << QString::number(142) << QString::number(135) << QString::number(131);

    m_viewColumnWidths = m_storage.readEntry("columnWidths", defaultcolumnwidths);
    m_viewSortColumn = m_storage.readEntry("sortColumn", (int)Model::Views::SCREENCOLUMN);
    m_viewSortOrder = static_cast<Qt::SortOrder>(m_storage.readEntry("sortOrder", (int)Qt::AscendingOrder));
}

void Views::saveConfig()
{
    m_storage.writeEntry("columnWidths", m_viewColumnWidths);
    m_storage.writeEntry("sortColumn", m_viewSortColumn);
    m_storage.writeEntry("sortOrder", (int)m_viewSortOrder);
}

}
}
}

