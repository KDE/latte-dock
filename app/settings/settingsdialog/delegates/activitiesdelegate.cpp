/*
    SPDX-FileCopyrightText: 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "activitiesdelegate.h"

// local
#include "../../generic/persistentmenu.h"
#include "../layoutsmodel.h"
#include "../../generic/generictools.h"
#include "../../../data/layoutdata.h"

// Qt
#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QMenu>
#include <QModelIndex>
#include <QPainter>
#include <QPushButton>
#include <QString>
#include <QTextDocument>
#include <QWidget>
#include <QWidgetAction>

// KDE
#include <KLocalizedString>

#define OKPRESSED "OKPRESSED"

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

Activities::Activities(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *Activities::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QPushButton *button = new QPushButton(parent);

    PersistentMenu *menu = new PersistentMenu(button);
    button->setMenu(menu);
    menu->setMinimumWidth(option.rect.width());

    bool isLayoutActive = index.data(Model::Layouts::ISACTIVEROLE).toBool();

    QStringList allActivities = index.data(Model::Layouts::ALLACTIVITIESSORTEDROLE).toStringList();
    Latte::Data::ActivitiesTable allActivitiesTable = index.data(Model::Layouts::ALLACTIVITIESDATAROLE).value<Latte::Data::ActivitiesTable>();

    QStringList assignedActivities = index.data(Qt::UserRole).toStringList();

    QList<int> originalChecked;

    QString currentrealactivityid;

    for (int i=0; i<allActivitiesTable.rowCount(); ++i) {
        if (allActivitiesTable[i].isCurrent) {
            currentrealactivityid = allActivitiesTable[i].id;
            break;
        }
    }

    for (int i=0; i<allActivities.count(); ++i) {
        Latte::Data::Activity activitydata = allActivitiesTable[allActivities[i]];

        if (!activitydata.isValid()) {
            continue;
        }


        bool inCurrentActivity = (activitydata.id == Data::Layout::CURRENTACTIVITYID && assignedActivities.contains(currentrealactivityid));
        bool ischecked = assignedActivities.contains(activitydata.id) || inCurrentActivity;

        if (ischecked) {
            originalChecked << i;
        }

        QAction *action = new QAction(activitydata.name);
        action->setData(activitydata.id);
        action->setIcon(QIcon::fromTheme(activitydata.icon));
        action->setCheckable(true);
        action->setChecked(ischecked);

        if (activitydata.id == Data::Layout::FREEACTIVITIESID
                || activitydata.id == Data::Layout::ALLACTIVITIESID
                || activitydata.id == Data::Layout::CURRENTACTIVITYID) {
            if (isLayoutActive) {
                QFont font = action->font();
                font.setBold(true);
                action->setFont(font);
            }

            if (ischecked) {
                menu->setMasterIndex(i);
            }

            connect(action, &QAction::toggled, this, [this, menu, button, action, i, allActivitiesTable, currentrealactivityid]() {
                if (action->isChecked()) {
                    menu->setMasterIndex(i);

                    if (action->data().toString() == Data::Layout::CURRENTACTIVITYID) {
                        auto actions = menu->actions();
                        for (int i=0; i<actions.count(); ++i) {
                            if (actions[i]->data().toString() == currentrealactivityid) {
                                actions[i]->setChecked(true);
                            }
                        }
                    }
                } else {
                    if (menu->masterIndex() == i) {
                        menu->setMasterIndex(-1);
                    }

                    if (action->data().toString() == Data::Layout::CURRENTACTIVITYID) {
                        auto actions = menu->actions();
                        for (int i=0; i<actions.count(); ++i) {
                            if (actions[i]->data().toString() == currentrealactivityid) {
                                actions[i]->setChecked(false);
                            }
                        }

                        updateCurrentActivityAction(menu);
                    }
                }              

                updateButton(button, allActivitiesTable);
            });            
        } else {
            if (activitydata.isRunning()) {
                QFont font = action->font();
                font.setBold(true);
                action->setFont(font);
            }

            connect(action, &QAction::toggled, this, [this, menu, button, action, i, allActivitiesTable]() {
                if (action->isChecked()) {
                    menu->setMasterIndex(-1);
                }

                updateButton(button, allActivitiesTable);
            });
        }

        menu->addAction(action);

        if (activitydata.id == Data::Layout::CURRENTACTIVITYID) {
            //! After CurrentActivity record we can add Separator
            menu->addSeparator();
        }
    }

    connect(menu, &PersistentMenu::masterIndexChanged, this, [this, menu, button, allActivitiesTable]() {
        int masterRow = menu->masterIndex();
        if (masterRow>=0) {
            auto actions = button->menu()->actions();

            for (int i=0; i<actions.count(); ++i) {
                if (i != masterRow && actions[i]->isChecked()) {
                    actions[i]->setChecked(false);
                }
            }
        } else {
            foreach (QAction *action, button->menu()->actions()) {
                QString actId = action->data().toString();
                if (actId == Data::Layout::FREEACTIVITIESID || actId == Data::Layout::ALLACTIVITIESID) {
                    action->setChecked(false);
                }
            }
        }

        updateCurrentActivityAction(menu);
        updateButton(button, allActivitiesTable);
    });

    //! Ok, Apply Buttons behavior
    menu->addSeparator();

    QDialogButtonBox *menuDialogButtons = new QDialogButtonBox(menu);
    menuDialogButtons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
    menuDialogButtons->setContentsMargins(3, 0, 3, 3);

    QWidgetAction* menuDialogButtonsWidgetAction = new QWidgetAction(menu);
    menuDialogButtonsWidgetAction->setDefaultWidget(menuDialogButtons);

    menu->addAction(menuDialogButtonsWidgetAction);

    connect(menuDialogButtons->button(QDialogButtonBox::Ok), &QPushButton::clicked,  [this, menu, button]() {
        button->setProperty(OKPRESSED, true);
        menu->hide();
    });

    connect(menuDialogButtons->button(QDialogButtonBox::Cancel), &QPushButton::clicked,  menu, &QMenu::hide);

    connect(menuDialogButtons->button(QDialogButtonBox::Reset), &QPushButton::clicked,  [this, menu, originalChecked]() {
        for (int i=0; i<menu->actions().count(); ++i) {
            if (!originalChecked.contains(i)) {
                menu->actions().at(i)->setChecked(false);
            } else {
                menu->actions().at(i)->setChecked(true);
            }
        }
    });

    connect(menu, &QMenu::aboutToHide, button, &QWidget::clearFocus);

    return button;
}

void Activities::updateCurrentActivityAction(QMenu *menu) const
{
    if (!menu) {
        return;
    }

    auto actions = menu->actions();
    for (int i=0; i<actions.count(); ++i) {
        if (actions[i]->data().toString() == Data::Layout::CURRENTACTIVITYID) {
            if (actions[i]->isChecked()) {
                QFont font = actions[i]->font();
                font.setBold(true);
                actions[i]->setFont(font);
            } else {
                QFont font = actions[i]->font();
                font.setBold(false);
                actions[i]->setFont(font);
            }
        }
    }

}

void Activities::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Latte::Data::ActivitiesTable allActivitiesTable = index.data(Model::Layouts::ALLACTIVITIESDATAROLE).value<Latte::Data::ActivitiesTable>();

    updateButton(editor, allActivitiesTable);
}

void Activities::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QPushButton *button = static_cast<QPushButton *>(editor);

    if (button->property(OKPRESSED).isNull() || !button->property(OKPRESSED).toBool()) {
        return;
    }

    //! keep activities that are present in other computers
    QStringList assignedActivities = index.data(Qt::UserRole).toStringList();

    foreach (QAction *action, button->menu()->actions()) {
        QString activityid = action->data().toString();

        if (activityid == Data::Layout::CURRENTACTIVITYID) {
            continue;
        }

        if (activityid == Data::Layout::ALLACTIVITIESID && action->isChecked()) {
            assignedActivities = QStringList(Data::Layout::ALLACTIVITIESID);
            break;
        } else if (activityid == Data::Layout::FREEACTIVITIESID && action->isChecked()) {
            assignedActivities = QStringList(Data::Layout::FREEACTIVITIESID);
            break;
        }

        //! try to not remove activityids that belong to different machines that are not
        //! currently present
        if (!action->isChecked()) {
            assignedActivities.removeAll(activityid);
        } else if (action->isChecked() && !assignedActivities.contains(activityid)) {
            assignedActivities << activityid;
        }
    }

    model->setData(index, assignedActivities, Qt::UserRole);
}

void Activities::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

bool Activities::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                             const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void Activities::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);

    bool isLayoutActive = index.data(Model::Layouts::ISACTIVEROLE).toBool();

    QList<Latte::Data::Activity> assignedActivities;
    QStringList assignedIds = index.model()->data(index, Qt::UserRole).toStringList();
    QStringList assignedOriginalIds = index.model()->data(index, Model::Layouts::ORIGINALASSIGNEDACTIVITIESROLE).toStringList();

    Latte::Data::ActivitiesTable allActivitiesTable = index.data(Model::Layouts::ALLACTIVITIESDATAROLE).value<Latte::Data::ActivitiesTable>();

    for (int i=0; i<assignedIds.count(); ++i) {
        assignedActivities << allActivitiesTable[assignedIds[i]];
    }

    if (assignedActivities.count() > 0) {
        myOptions.text = joinedActivities(assignedActivities, assignedOriginalIds, isLayoutActive);
    } else {
        myOptions.text = "";
    }

    Latte::drawBackground(painter, option);
    Latte::drawFormattedText(painter, myOptions);
}

QString Activities::joinedActivities(const QList<Latte::Data::Activity> &activities, const QStringList &originalIds, bool isActive, bool formatText) const
{
    QString finalText;

    int i = 0;

    for (int i=0; i<activities.count(); ++i) {
        bool bold{false};
        bool italic = (!originalIds.contains(activities[i].id));

        if (activities[i].id == Data::Layout::FREEACTIVITIESID || activities[i].id == Data::Layout::ALLACTIVITIESID) {
            bold = isActive;
        } else {
            bold = activities[i].isRunning();
        }

        if (i > 0) {
            finalText += ", ";
        }

        QString styledText = activities[i].name;

        if (bold && formatText) {
            styledText = "<b>" + styledText + "</b>";
        }

        if (italic && formatText) {
            styledText = "<i>" + styledText + "</i>";
        }

        finalText += styledText;
    }

    return finalText;
}

void Activities::updateButton(QWidget *editor, const Latte::Data::ActivitiesTable &allActivitiesTable) const
{
    if (!editor) {
        return;
    }

    QPushButton *button = static_cast<QPushButton *>(editor);
    QList<Latte::Data::Activity> assignedActivities;

    foreach (QAction *action, button->menu()->actions()) {
        if (action->isChecked() && action->data().toString() != Data::Layout::CURRENTACTIVITYID) {
            assignedActivities << allActivitiesTable[action->data().toString()];
        }
    }

    button->setText(joinedActivities(assignedActivities, QStringList(), false, false));
}

}
}
}
}

