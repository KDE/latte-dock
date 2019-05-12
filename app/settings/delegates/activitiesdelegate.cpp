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
#include "../settingsdialog.h"
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

ActivitiesDelegate::ActivitiesDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    auto *settingsDialog = qobject_cast<Latte::SettingsDialog *>(parent);

    if (settingsDialog) {
        m_settingsDialog = settingsDialog;
    }
}

QWidget *ActivitiesDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QPushButton *button = new QPushButton(parent);
    PersistentMenu *menu = new PersistentMenu(button);
    button->setMenu(menu);
    menu->setMinimumWidth(option.rect.width());

    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();
    QStringList availableActivities = m_settingsDialog->availableActivities();
    QStringList activities = m_settingsDialog->activities();

    QStringList shownActivities;

    for (const auto &activity : activities) {
        if (assignedActivities.contains(activity) || availableActivities.contains(activity)) {
            shownActivities.append(activity);
        }
    }

    for (unsigned int i = 0; i < shownActivities.count(); ++i) {
        KActivities::Info info(shownActivities[i]);

        if (info.state() != KActivities::Info::Invalid) {
            QAction *action = new QAction(info.name());
            action->setData(shownActivities[i]);
            action->setIcon(QIcon::fromTheme(info.icon()));
            action->setCheckable(true);
            action->setChecked(assignedActivities.contains(shownActivities[i]));

            if ((info.state() == KActivities::Info::Running) || (info.state() == KActivities::Info::Starting)) {
                QFont font = action->font();
                font.setBold(true);
                action->setFont(font);
            }

            menu->addAction(action);

            connect(action, &QAction::toggled, this, [this, button, action]() {
                updateButton(button);

                if (action->isChecked()) {
                    m_settingsDialog->addActivityInCurrent(action->data().toString());
                } else {
                    m_settingsDialog->removeActivityFromCurrent(action->data().toString());
                }
            });
        }
    }

    return button;
}

void ActivitiesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    updateButton(editor);
}

void ActivitiesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
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

void ActivitiesDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ActivitiesDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);

    if (myOptions.state & QStyle::State_Enabled) {
        painter->save();

        QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();

        if (assignedActivities.count() > 0) {
            myOptions.text = joinedActivities(assignedActivities);

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
        QPalette palette;
        QPen pen(Qt::DashDotDotLine);

        pen.setWidth(2); pen.setColor(palette.text().color());
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
            painter->setBrush(palette.text().color());

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
            painter->setBrush(palette.text().color());

            //! draw ending cirlce
            painter->drawEllipse(QPoint(xm, ym + thick/2), thick/4, thick/4);
        }
    }
}

QString ActivitiesDelegate::joinedActivities(const QStringList &activities, bool boldForActive) const
{
    QString finalText;

    int i = 0;

    for (const auto &activityId : activities) {
        KActivities::Info info(activityId);

        if (info.state() != KActivities::Info::Invalid) {
            if (i > 0) {
                finalText += ", ";
            }
            i++;

            bool isActive{false};

            if (boldForActive && (info.state() == KActivities::Info::Running) || (info.state() == KActivities::Info::Starting)) {
                isActive = true;
            }

            finalText += isActive ? "<b>" + info.name() + "</b>" : info.name();
        }
    }

    return finalText;
}

void ActivitiesDelegate::updateButton(QWidget *editor) const
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

    button->setText(joinedActivities(assignedActivities,false));
}

