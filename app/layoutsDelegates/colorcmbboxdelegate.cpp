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

#include "colorcmbboxdelegate.h"
#include "colorcmbboxitemdelegate.h"

#include "../settingsdialog.h"

#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QWidget>
#include <QModelIndex>
#include <QApplication>
#include <QPainter>
#include <QString>

#include <KLocalizedString>

ColorCmbBoxDelegate::ColorCmbBoxDelegate(QObject *parent, QString iconsPath, QStringList colors)
    : QItemDelegate(parent),
      m_parent(parent),
      m_iconsPath(iconsPath),
      Colors(colors)
{
}

QWidget *ColorCmbBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->setItemDelegate(new ColorCmbBoxItemDelegate(editor, m_iconsPath));

    for (unsigned int i = 0; i < Colors.count(); ++i) {
        if (Colors[i] != "sepia") {
            QPixmap pixmap(50, 50);
            pixmap.fill(QColor(Colors[i]));
            QIcon icon(pixmap);

            editor->addItem(icon, Colors[i]);
        }
    }

    QString value = index.model()->data(index, Qt::BackgroundRole).toString();

    const QModelIndex &indexOriginal = index;

    bool showTextColor{false};

    //! add the background if exists
    if (value.startsWith("/")) {
        QIcon icon(value);
        editor->addItem(icon, value);
        showTextColor = true;
    }

    editor->addItem(" " + i18n("Select image..."), "select_image");

    if (showTextColor) {
        editor->addItem(" " + i18n("Text color..."), "text_color");
    }

    connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), [ = ](int index) {
        editor->clearFocus();

        if ((showTextColor && index == editor->count() - 2)
            || (!showTextColor && index == editor->count() - 1)) {
            Latte::SettingsDialog *settings = qobject_cast<Latte::SettingsDialog *>(m_parent);

            if (settings) {
                settings->requestImagesDialog(indexOriginal.row());
            }
        } else if (showTextColor && index == editor->count() - 1) {
            Latte::SettingsDialog *settings = qobject_cast<Latte::SettingsDialog *>(m_parent);

            if (settings) {
                settings->requestColorsDialog(indexOriginal.row());
            }
        }
    });

    return editor;
}

void ColorCmbBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    QString value = index.model()->data(index, Qt::BackgroundRole).toString();
    QString userData = index.model()->data(index, Qt::UserRole).toString();

    int pos = Colors.indexOf(value);

    if (pos == -1 && value.startsWith("/")) {
        comboBox->setCurrentIndex(Colors.count());
    } else {
        comboBox->setCurrentIndex(Colors.indexOf(value));
    }
}

void ColorCmbBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);

    QString itemData = comboBox->currentData().toString();

    if (itemData != "select_image" && itemData != "text_color") {
        model->setData(index, comboBox->currentText(), Qt::BackgroundRole);
    }
}

void ColorCmbBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ColorCmbBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    QVariant value = index.data(Qt::BackgroundRole);
    QVariant data = index.data(Qt::UserRole);
    QString dataStr = data.toString();

    if (value.isValid() && (dataStr != "select_image") && (dataStr != "text_color")) {
        QString valueStr = value.toString();

        QString colorPath = valueStr.startsWith("/") ? valueStr : m_iconsPath + value.toString() + "print.jpg";

        if (QFileInfo(colorPath).exists()) {
            QBrush colorBrush;
            colorBrush.setTextureImage(QImage(colorPath).scaled(QSize(50, 50)));

            painter->setBrush(colorBrush);
            painter->drawRect(QRect(option.rect.x(), option.rect.y(),
                                    option.rect.width(), option.rect.height()));
        }
    }

    //QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}

