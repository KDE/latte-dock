/*
*  Copyright 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "activitiesdelegate.h"

// local
#include "persistentmenu.h"
#include "../data/activitydata.h"
#include "../data/layoutdata.h"
#include "../models/layoutsmodel.h"
#include "../tools/settingstools.h"

// Qt
#include <QApplication>
#include <QDebug>
#include <QWidget>
#include <QMenu>
#include <QModelIndex>
#include <QPainter>
#include <QPushButton>
#include <QString>
#include <QTextDocument>


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

    bool isLayoutActive = index.data(Model::Layouts::LAYOUTISACTIVEROLE).toBool();

    QStringList allActivities = index.data(Model::Layouts::ALLACTIVITIESSORTEDROLE).toStringList();
    Data::ActivitiesMap allActivitiesData = index.data(Model::Layouts::ALLACTIVITIESDATAROLE).value<Data::ActivitiesMap>();

    QStringList assignedActivities = index.data(Qt::UserRole).toStringList();

    for (int i = 0; i < allActivities.count(); ++i) {
        Data::Activity activitydata = allActivitiesData[allActivities[i]];

        if (!activitydata.isValid()) {
            continue;
        }

        bool ischecked = assignedActivities.contains(activitydata.id);

        QAction *action = new QAction(activitydata.name);
        action->setData(activitydata.id);
        action->setIcon(QIcon::fromTheme(activitydata.icon));
        action->setCheckable(true);
        action->setChecked(ischecked);

        if (activitydata.id == Data::Layout::FREEACTIVITIESID) {
            if (isLayoutActive) {
                QFont font = action->font();
                font.setBold(true);
                action->setFont(font);
            }

            if (ischecked) {
                menu->setMasterIndex(i);
            }

            connect(action, &QAction::toggled, this, [this, menu, button, action, i, allActivitiesData]() {
                if (action->isChecked()) {
                    menu->setMasterIndex(i);
                } else {
                    if (menu->masterIndex() == i) {
                        action->setChecked(true);
                    }
                }

                updateButton(button, allActivitiesData);
            });
        } else {
            if (activitydata.isRunning()) {
                QFont font = action->font();
                font.setBold(true);
                action->setFont(font);
            }

            connect(action, &QAction::toggled, this, [this, menu, button, action, i, allActivitiesData]() {
                if (action->isChecked()) {
                    menu->setMasterIndex(-1);
                }

                updateButton(button, allActivitiesData);
            });
        }

        menu->addAction(action);
    }

    connect(menu, &PersistentMenu::masterIndexChanged, this, [this, menu, button, allActivitiesData]() {
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
                if (actId == Data::Layout::FREEACTIVITIESID) {
                    action->setChecked(false);
                }
            }
        }

        updateButton(button, allActivitiesData);
    });

    return button;
}

void Activities::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Data::ActivitiesMap allActivitiesData = index.data(Model::Layouts::ALLACTIVITIESDATAROLE).value<Data::ActivitiesMap>();

    updateButton(editor, allActivitiesData);
}

void Activities::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QPushButton *button = static_cast<QPushButton *>(editor);

    QStringList assignedActivities;
    foreach (QAction *action, button->menu()->actions()) {
        if (action->isChecked()) {
            assignedActivities << action->data().toString();
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

    bool isSharedCapable = index.data(Model::Layouts::LAYOUTISSHAREDROLE).toBool() && index.data(Model::Layouts::INMULTIPLELAYOUTSROLE).toBool();

    if (isSharedCapable) {
        return false;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void Activities::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);

    bool isLayoutActive = index.data(Model::Layouts::LAYOUTISACTIVEROLE).toBool();
    bool isSharedCapable = index.data(Model::Layouts::LAYOUTISSHAREDROLE).toBool() && index.data(Model::Layouts::INMULTIPLELAYOUTSROLE).toBool();

    if (!isSharedCapable) {
        painter->save();

        QList<Data::Activity> assignedActivities;
        QStringList assignedIds = index.model()->data(index, Qt::UserRole).toStringList();

        Data::ActivitiesMap allActivitiesData = index.data(Model::Layouts::ALLACTIVITIESDATAROLE).value<Data::ActivitiesMap>();

        for (int i=0; i<assignedIds.count(); ++i) {
            assignedActivities << allActivitiesData[assignedIds[i]];
        }

        if (assignedActivities.count() > 0) {
            myOptions.text = joinedActivities(assignedActivities, isLayoutActive);

            QTextDocument doc;
            QString css;
            QString activitiesText = myOptions.text;

            QPalette::ColorRole applyColor = Latte::isSelected(option) ? QPalette::HighlightedText : QPalette::Text;
            QBrush nBrush = option.palette.brush(Latte::colorGroup(option), applyColor);

            css = QString("body { color : %1; }").arg(nBrush.color().name());

            doc.setDefaultStyleSheet(css);
            doc.setHtml("<body>" + myOptions.text + "</body>");

            myOptions.text = "";
            myOptions.widget->style()->drawControl(QStyle::CE_ItemViewItem, &myOptions, painter);

            //we need an offset to be in the same vertical center of TextEdit
            int offsetY = ((myOptions.rect.height() - doc.size().height()) / 2);

            if ((qApp->layoutDirection() == Qt::RightToLeft) && !activitiesText.isEmpty()) {
                int textWidth = doc.size().width();

                painter->translate(qMax(myOptions.rect.left(), myOptions.rect.right() - textWidth), myOptions.rect.top() + offsetY + 1);
            } else {
                painter->translate(myOptions.rect.left(), myOptions.rect.top() + offsetY + 1);
            }

            QRect clip(0, 0, myOptions.rect.width(), myOptions.rect.height());
            doc.drawContents(painter, clip);
        } else {
            QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOptions, painter);
        }

        painter->restore();
    } else {
        bool sharedInEdit = index.data(Model::Layouts::SHAREDTOINEDIT).toBool();

        // Disabled
        bool isSelected{Latte::isSelected(option)};
        QPalette::ColorRole backColorRole = isSelected ? QPalette::Highlight : QPalette::Base;
        QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;

        //! draw background
        //! HIDDENTEXTCOLUMN is just needed to draw empty background rectangles properly based on states
        QStyledItemDelegate::paint(painter, option, index.model()->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));

        // text
        QPen pen(Qt::DotLine);
        QColor textColor = option.palette.brush(Latte::colorGroup(option), textColorRole).color();

        pen.setWidth(2); pen.setColor(textColor);
        int y = option.rect.y()+option.rect.height()/2;

        int space = option.rect.height() / 2;

        painter->setPen(pen);

        if (sharedInEdit) {
            //! shareto cell is in edit mode so circle indicator is moved inside
            //! the activities cell
            if (qApp->layoutDirection() == Qt::LeftToRight) {
                painter->drawLine(option.rect.x(), y, option.rect.x()+option.rect.width() - space, y);

                int xm = option.rect.x() + option.rect.width() - space;
                int thick = option.rect.height() / 2;
                int ym = option.rect.y() + ((option.rect.height() - thick) / 2);

                pen.setStyle(Qt::SolidLine);
                painter->setPen(pen);
                painter->setBrush(textColor);

                //! draw ending cirlce
                painter->drawEllipse(QPoint(xm, ym + thick/2), thick/4, thick/4);
            } else {
                painter->drawLine(option.rect.x() + space, y, option.rect.x() + option.rect.width(), y);

                int xm = option.rect.x() + space;
                int thick = option.rect.height() / 2;
                int ym = option.rect.y() + ((option.rect.height() - thick) / 2);

                pen.setStyle(Qt::SolidLine);
                painter->setPen(pen);
                painter->setBrush(textColor);

                //! draw ending cirlce
                painter->drawEllipse(QPoint(xm, ym + thick/2), thick/4, thick/4);
            }
        } else {
                painter->drawLine(option.rect.x(), y, option.rect.x()+option.rect.width(), y);
        }
    }
}

QString Activities::joinedActivities(const QList<Data::Activity> &activities, bool isActive, bool formatText) const
{
    QString finalText;

    int i = 0;

    for (int i=0; i<activities.count(); ++i) {
        bool bold{false};
        bool italic{false};

        if (activities[i].id == Data::Layout::FREEACTIVITIESID) {
            if (formatText) {
                bold = isActive;
                italic = !isActive;
            }
        } else {
            if (formatText && activities[i].isRunning()) {
                bold = true;
            }
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

void Activities::updateButton(QWidget *editor, const Data::ActivitiesMap &allActivitiesData) const
{
    if (!editor) {
        return;
    }

    QPushButton *button = static_cast<QPushButton *>(editor);
    QList<Data::Activity> assignedActivities;

    foreach (QAction *action, button->menu()->actions()) {
        if (action->isChecked()) {
            assignedActivities << allActivitiesData[action->data().toString()];
        }
    }

    button->setText(joinedActivities(assignedActivities, false, false));
}

}
}
}
}

