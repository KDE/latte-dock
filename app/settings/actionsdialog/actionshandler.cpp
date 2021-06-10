/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "actionshandler.h"

// local
#include "ui_actionsdialog.h"
#include "actionsdialog.h"
#include "../settingsdialog/tabpreferenceshandler.h"
#include "../../data/contextmenudata.h"

// Qt
#include <QDebug>

// KDE
#include <KActionSelector>
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Handler {

ActionsHandler::ActionsHandler(Dialog::ActionsDialog *dialog)
    : Generic(dialog),
      m_dialog(dialog),
      m_ui(m_dialog->ui())
{
    initItems();
    init();
}

ActionsHandler::~ActionsHandler()
{
}

void ActionsHandler::init()
{
    connect(m_ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ActionsHandler::onCancel);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ActionsHandler::save);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &ActionsHandler::reset);
    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &ActionsHandler::resetDefaults);

    connect(m_ui->actionsSelector, &KActionSelector::added, this, [&]() {
        updateButtonEnablement();
    });

    connect(m_ui->actionsSelector, &KActionSelector::removed, this, [&]() {
        updateButtonEnablement();
    });
}

void ActionsHandler::initItems()
{
    o_alwaysActions = m_dialog->preferencesHandler()->contextMenuAlwaysActions();

    QString itemid = Latte::Data::ContextMenu::LAYOUTSACTION;
    int itemindex = Latte::Data::ContextMenu::ACTIONSEDITORDER.indexOf(itemid);
    m_items[itemid] = new ActionsDialog::ActionListWidgetItem(QIcon::fromTheme("user-identity"),
                                                              i18n("Layouts"),
                                                              itemindex,
                                                              itemid);

    itemid = Latte::Data::ContextMenu::PREFERENCESACTION;
    itemindex = Latte::Data::ContextMenu::ACTIONSEDITORDER.indexOf(itemid);
    m_items[itemid] = new ActionsDialog::ActionListWidgetItem(QIcon::fromTheme("configure"),
                                                              i18nc("global settings window", "Configure Latte..."),
                                                              itemindex,
                                                              itemid);

    itemid = Latte::Data::ContextMenu::QUITLATTEACTION;
    itemindex = Latte::Data::ContextMenu::ACTIONSEDITORDER.indexOf(itemid);
    m_items[itemid] = new ActionsDialog::ActionListWidgetItem(QIcon::fromTheme("application-exit"),
                                                              i18nc("quit application", "Quit Latte"),
                                                              itemindex,
                                                              itemid);

    itemid = Latte::Data::ContextMenu::SEPARATOR1ACTION;
    itemindex = Latte::Data::ContextMenu::ACTIONSEDITORDER.indexOf(itemid);
    m_items[itemid] = new ActionsDialog::ActionListWidgetItem(QIcon::fromTheme(""),
                                                              i18n(" --- separator --- "),
                                                              itemindex,
                                                              itemid);

    itemid = Latte::Data::ContextMenu::ADDWIDGETSACTION;
    itemindex = Latte::Data::ContextMenu::ACTIONSEDITORDER.indexOf(itemid);
    m_items[itemid] = new ActionsDialog::ActionListWidgetItem(QIcon::fromTheme("list-add"),
                                                              i18n("Add Widgets..."),
                                                              itemindex,
                                                              itemid);

    itemid = Latte::Data::ContextMenu::ADDVIEWACTION;
    itemindex = Latte::Data::ContextMenu::ACTIONSEDITORDER.indexOf(itemid);
    m_items[itemid] = new ActionsDialog::ActionListWidgetItem(QIcon::fromTheme("list-add"),
                                                              i18n("Add Dock/Panel"),
                                                              itemindex,
                                                              itemid);

    itemid = Latte::Data::ContextMenu::MOVEVIEWACTION;
    itemindex = Latte::Data::ContextMenu::ACTIONSEDITORDER.indexOf(itemid);
    m_items[itemid] = new ActionsDialog::ActionListWidgetItem(QIcon::fromTheme("transform-move-horizontal"),
                                                              i18n("Move Dock/Panel To Layout"),
                                                              itemindex,
                                                              itemid);

    itemid = Latte::Data::ContextMenu::EXPORTVIEWTEMPLATEACTION;
    itemindex = Latte::Data::ContextMenu::ACTIONSEDITORDER.indexOf(itemid);
    m_items[itemid] = new ActionsDialog::ActionListWidgetItem(QIcon::fromTheme("document-export"),
                                                              i18n("Export Dock/Panel as Template..."),
                                                              itemindex,
                                                              itemid);

    itemid = Latte::Data::ContextMenu::REMOVEVIEWACTION;
    itemindex = Latte::Data::ContextMenu::ACTIONSEDITORDER.indexOf(itemid);
    m_items[itemid] = new ActionsDialog::ActionListWidgetItem(QIcon::fromTheme("delete"),
                                                              i18n("Remove Dock/Panel"),
                                                              itemindex,
                                                              itemid);

    loadItems(m_dialog->preferencesHandler()->contextMenuAlwaysActions());
}

void ActionsHandler::loadItems(const QStringList &alwaysActions)
{
    for(int i=0; i<Latte::Data::ContextMenu::ACTIONSEDITORDER.count(); ++i) {
        QString actionname = Latte::Data::ContextMenu::ACTIONSEDITORDER[i];
        bool inalways = alwaysActions.contains(actionname);

        int rowinalways = m_ui->actionsSelector->selectedListWidget()->row(m_items[actionname]);
        int rowinedit = m_ui->actionsSelector->availableListWidget()->row(m_items[actionname]);

        if (inalways && rowinalways == -1) {
            if (rowinedit >= 0) {
                m_ui->actionsSelector->availableListWidget()->takeItem(rowinedit);
            }
            m_ui->actionsSelector->selectedListWidget()->addItem(m_items[actionname]);
        } else if (!inalways && rowinedit == -1) {
            if (rowinalways >= 0) {
                m_ui->actionsSelector->selectedListWidget()->takeItem(rowinalways);
            }
            m_ui->actionsSelector->availableListWidget()->addItem(m_items[actionname]);
        }
    }

    m_ui->actionsSelector->setAvailableInsertionPolicy(KActionSelector::Sorted);
    m_ui->actionsSelector->setSelectedInsertionPolicy(KActionSelector::Sorted);

    m_ui->actionsSelector->availableListWidget()->sortItems();
    m_ui->actionsSelector->selectedListWidget()->sortItems();

    updateButtonEnablement();
}

bool ActionsHandler::hasChangedData() const
{
    return currentAlwaysData() != o_alwaysActions;
}

bool ActionsHandler::inDefaultValues() const
{
    return currentAlwaysData() == Data::ContextMenu::ACTIONSALWAYSVISIBLE;
}

Data::GenericTable<Data::Generic> ActionsHandler::table(const QStringList &ids)
{
    Data::GenericTable<Data::Generic> bastable;

    for(int i=0; i<ids.count(); ++i) {
        bastable << Data::Generic(ids[i], "");
    }

    return bastable;
}

QStringList ActionsHandler::currentAlwaysData() const
{
    QStringList always;

    for(int i=0; i<m_ui->actionsSelector->selectedListWidget()->count(); ++i) {
        QListWidgetItem *widgetitem = m_ui->actionsSelector->selectedListWidget()->item(i);
        always << widgetitem->data(ActionsDialog::ActionListWidgetItem::IDROLE).toString();
    }

    return always;
}

void ActionsHandler::reset()
{
    loadItems(o_alwaysActions);
}

void ActionsHandler::resetDefaults()
{
    loadItems(Data::ContextMenu::ACTIONSALWAYSVISIBLE);
}

void ActionsHandler::updateButtonEnablement()
{
    bool haschanges = hasChangedData();
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(haschanges);
    m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(haschanges);
    m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(!inDefaultValues());
}

void ActionsHandler::onCancel()
{
    qDebug() << Q_FUNC_INFO;
    m_dialog->close();
}

void ActionsHandler::save()
{
    qDebug() << Q_FUNC_INFO;
    m_dialog->preferencesHandler()->setContextMenuAlwaysActions(currentAlwaysData());
    m_dialog->close();
}

}
}
}
