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
    int row = index.row();
    QPushButton *button = new QPushButton(parent);

    PersistentMenu *menu = new PersistentMenu(button);
    button->setMenu(menu);
    menu->setMinimumWidth(option.rect.width());

    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();
    QStringList shownActivities = m_settingsDialog->activitiesList();

    m_settingsDialog->loadActivitiesInBuffer(row);

    QString freeActivitiesId = m_settingsDialog->freeActivities_id();

    for (unsigned int i = 0; i < shownActivities.count(); ++i) {

        if (shownActivities[i] == freeActivitiesId) {
            bool isFreeActivitiesChecked = assignedActivities.contains(freeActivitiesId);

            QAction *action = new QAction(m_settingsDialog->freeActivities_text());
            action->setData(freeActivitiesId);
            action->setIcon(QIcon::fromTheme(m_settingsDialog->freeActivities_icon()));
            action->setCheckable(true);
            action->setChecked(isFreeActivitiesChecked);

            bool isActive = m_settingsDialog->isActive(row);

            if (isActive) {
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
                    m_settingsDialog->addActivityInBuffer(action->data().toString());
                    menu->setMasterIndex(i);
                } else {
                    if (menu->masterIndex() == i) {
                        action->setChecked(true);
                    }
                    //do nothing....
                    //m_settingsDialog->removeActivityFromBuffer(action->data().toString());
                }

                updateButton(button);
            });
        } else {
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

                connect(action, &QAction::toggled, this, [this, menu, button, action, i]() {
                    if (action->isChecked()) {
                        menu->setMasterIndex(-1);
                        m_settingsDialog->addActivityInBuffer(action->data().toString());
                    } else {
                        m_settingsDialog->removeActivityFromBuffer(action->data().toString());
                    }

                    updateButton(button);
                });
            }
        }
    }

    connect(menu, &PersistentMenu::masterIndexChanged, this, [this, menu, button, freeActivitiesId]() {
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
                if (actId == freeActivitiesId) {
                    action->setChecked(false);
                }
            }
        }

        updateButton(button);
    });

    connect(menu, &QMenu::aboutToHide, this, [this, row]() {
        m_settingsDialog->syncActivitiesFromBuffer(row);
    });

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
            myOptions.text = joinedActivities(assignedActivities, index.row());

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

QString ActivitiesDelegate::joinedActivities(const QStringList &activities, int index) const
{
    QString finalText;

    int i = 0;

    QString freeActivitiesId = m_settingsDialog->freeActivities_id();

    for (const auto &activityId : activities) {
        QString name;
        bool bold{false};
        bool italic{false};

        if (activityId == freeActivitiesId) {
            name = m_settingsDialog->freeActivities_text();

            if (index >= 0) {
                bool isActive = m_settingsDialog->isActive(index);
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

                if ((info.state() == KActivities::Info::Running) || (info.state() == KActivities::Info::Starting)) {
                    bold = true;
                }

                name = info.name();
            }
        }

        QString styledText = name;

        if (bold) {
            styledText = "<b>" + styledText + "</b>";
        }

        if (italic) {
            styledText = "<i>" + styledText + "</i>";
        }

        finalText += styledText;
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

    button->setText(joinedActivities(assignedActivities, -1));
}

