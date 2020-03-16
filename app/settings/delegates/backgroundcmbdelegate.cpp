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

#include "backgroundcmbdelegate.h"

// local
#include "backgroundcmbitemdelegate.h"
#include "../models/layoutsmodel.h"
#include "../tools/settingstools.h"

// Qt
#include <QComboBox>
#include <QDebug>
#include <QFileInfo>
#include <QWidget>
#include <QModelIndex>
#include <QPainter>
#include <QString>

// KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

BackgroundCmbBox::BackgroundCmbBox(QObject *parent, QString iconsPath, QStringList colors)
    : QStyledItemDelegate(parent),
      m_iconsPath(iconsPath),
      Colors(colors)
{
}

QWidget *BackgroundCmbBox::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    QComboBox *editor = new QComboBox(parent);
    editor->setItemDelegate(new BackgroundCmbBoxItem(editor, m_iconsPath));

    for (int i = 0; i < Colors.count(); ++i) {
        if (Colors[i] != "sepia") {
            QPixmap pixmap(50, 50);
            pixmap.fill(QColor(Colors[i]));
            QIcon icon(pixmap);

            editor->addItem(icon, Colors[i]);
        }
    }

    QString value = index.model()->data(index, Qt::BackgroundRole).toString();

    //! add the background if exists
    if (value.startsWith("/")) {
        QIcon icon(value);
        editor->addItem(icon, value);
    }

    return editor;
}

void BackgroundCmbBox::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    QString value = index.model()->data(index, Qt::BackgroundRole).toString();

    int pos = Colors.indexOf(value);

    if (pos == -1 && value.startsWith("/")) {
        comboBox->setCurrentIndex(Colors.count());
    } else {
        comboBox->setCurrentIndex(Colors.indexOf(value));
    }
}

void BackgroundCmbBox::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{   
    QComboBox *comboBox = static_cast<QComboBox *>(editor);

    QString itemData = comboBox->currentData().toString();
    model->setData(index, comboBox->currentText(), Qt::BackgroundRole);
}

void BackgroundCmbBox::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}

void BackgroundCmbBox::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);

    QVariant background = index.data(Qt::BackgroundRole);

    //! draw underlying background
    QStyledItemDelegate::paint(painter, myOptions, index);

    if (background.isValid()) {
        QString backgroundStr = background.toString();

        QString colorPath = backgroundStr.startsWith("/") ? backgroundStr : m_iconsPath + backgroundStr + "print.jpg";

        if (QFileInfo(colorPath).exists()) {
            bool isSelected{Latte::isSelected(option)};
            QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;

            QPen pen;

            QBrush colorBrush;
            colorBrush.setTextureImage(QImage(colorPath));

            pen.setWidth(1);
            pen.setColor(option.palette.color(Latte::colorGroup(option), textColorRole));

            painter->setPen(pen);
            painter->setBrush(colorBrush);

            int cX = option.rect.x() + (option.rect.width() / 2);
            int cY = option.rect.y() + (option.rect.height() / 2);
            int radius = (option.rect.height() - 4) / 2;

            painter->drawEllipse(QPointF(cX, cY), radius, radius);
        }
    }
}

}
}
}
}

