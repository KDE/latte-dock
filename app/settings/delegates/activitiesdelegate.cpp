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

// KDE
#include <KActivities/Info>
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

Activities::Activities(QObject *parent)
    : QItemDelegate(parent)
{
}

QString Activities::freeActivities_text() const
{
    return QString("[ " + i18n("All Free Activities...") + " ]");
}

QString Activities::freeActivities_icon() const
{
    return "favorites";
}

QWidget *Activities::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QPushButton *button = new QPushButton(parent);

    PersistentMenu *menu = new PersistentMenu(button);
    button->setMenu(menu);
    menu->setMinimumWidth(option.rect.width());

    bool isLayoutActive = index.data(Model::Layouts::LAYOUTISACTIVEROLE).toBool();
    QStringList allActivities = index.data(Model::Layouts::ALLACTIVITIESROLE).toStringList();
    QStringList assignedActivities = index.data(Qt::UserRole).toStringList();

    for (int i = 0; i < allActivities.count(); ++i) {
        if (allActivities[i] == Model::Layouts::FREEACTIVITIESID) {
            bool isFreeActivitiesChecked = assignedActivities.contains(Model::Layouts::FREEACTIVITIESID);

            QAction *action = new QAction(freeActivities_text());
            action->setData(Model::Layouts::FREEACTIVITIESID);
            action->setIcon(QIcon::fromTheme(freeActivities_icon()));
            action->setCheckable(true);
            action->setChecked(isFreeActivitiesChecked);

            if (isLayoutActive) {
                QFont font = action->font();
                font.setBold(true);
                action->setFont(font);
            }

            menu->addAction(action);
            if (isFreeActivitiesChecked) {
                menu->setMasterIndex(i);
            }

            connect(action, &QAction::toggled, this, [this, menu, button, action, i]() {
                if (action->isChecked()) {
                    menu->setMasterIndex(i);
                } else {
                    if (menu->masterIndex() == i) {
                        action->setChecked(true);
                    }
                }

                updateButton(button);
            });
        } else {
            KActivities::Info info(allActivities[i]);

            if (info.state() != KActivities::Info::Invalid) {
                QAction *action = new QAction(info.name());
                action->setData(allActivities[i]);
                action->setIcon(QIcon::fromTheme(info.icon()));
                action->setCheckable(true);
                action->setChecked(assignedActivities.contains(allActivities[i]));

                if ((info.state() == KActivities::Info::Running) || (info.state() == KActivities::Info::Starting)) {
                    QFont font = action->font();
                    font.setBold(true);
                    action->setFont(font);
                }

                menu->addAction(action);

                connect(action, &QAction::toggled, this, [this, menu, button, action, i]() {
                    if (action->isChecked()) {
                        menu->setMasterIndex(-1);
                    }

                    updateButton(button);
                });
            }
        }
    }

    connect(menu, &PersistentMenu::masterIndexChanged, this, [this, menu, button]() {
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
                if (actId == Model::Layouts::FREEACTIVITIESID) {
                    action->setChecked(false);
                }
            }
        }

        updateButton(button);
    });

    return button;
}

void Activities::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    updateButton(editor);
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

    return QItemDelegate::editorEvent(event, model, option, index);
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

        QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();

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
        // Disabled
        bool isSelected{Latte::isSelected(option)};
        QPalette::ColorRole backColorRole = isSelected ? QPalette::Highlight : QPalette::Base;
        QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;

        // background
        painter->fillRect(option.rect, option.palette.brush(Latte::colorGroup(option), backColorRole));

        // text
        QPen pen(Qt::DashDotDotLine);
        QColor textColor = option.palette.brush(Latte::colorGroup(option), textColorRole).color();

        pen.setWidth(2); pen.setColor(textColor);
        int y = option.rect.y()+option.rect.height()/2;

        int space = option.rect.height() / 2;

        painter->setPen(pen);

        if (qApp->layoutDirection() == Qt::LeftToRight) {
            painter->drawLine(option.rect.x(), y,
                              option.rect.x()+option.rect.width() - space, y);

            int xm = option.rect.x() + option.rect.width() - space;
            int thick = option.rect.height() / 2;
            int ym = option.rect.y() + ((option.rect.height() - thick) / 2);

            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            painter->setBrush(textColor);

            //! draw ending cirlce
            painter->drawEllipse(QPoint(xm, ym + thick/2), thick/4, thick/4);
        } else {
            painter->drawLine(option.rect.x() + space, y,
                              option.rect.x() + option.rect.width(), y);

            int xm = option.rect.x() + space;
            int thick = option.rect.height() / 2;
            int ym = option.rect.y() + ((option.rect.height() - thick) / 2);

            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            painter->setBrush(textColor);

            //! draw ending cirlce
            painter->drawEllipse(QPoint(xm, ym + thick/2), thick/4, thick/4);
        }
    }
}

QString Activities::joinedActivities(const QStringList &activities, bool isActive, bool formatText) const
{
    QString finalText;

    int i = 0;

    for (const auto &activityId : activities) {
        QString name;
        bool bold{false};
        bool italic{false};

        if (activityId == Model::Layouts::FREEACTIVITIESID) {
            name = freeActivities_text();

            if (formatText) {
                bold = isActive;
                italic = !isActive;
            }
        } else {
            KActivities::Info info(activityId);

            if (info.state() != KActivities::Info::Invalid) {
                if (i > 0) {
                    finalText += ", ";
                }
                i++;

                if (formatText && ((info.state() == KActivities::Info::Running) || (info.state() == KActivities::Info::Starting))) {
                    bold = true;
                }

                name = info.name();
            }
        }

        QString styledText = name;

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

void Activities::updateButton(QWidget *editor) const
{
    if (!editor) {
        return;
    }
    QPushButton *button = static_cast<QPushButton *>(editor);
    QStringList assignedActivities;

    foreach (QAction *action, button->menu()->actions()) {
        if (action->isChecked()) {
            assignedActivities << action->data().toString();
        }
    }

    button->setText(joinedActivities(assignedActivities, false, false));
}

}
}
}
}

