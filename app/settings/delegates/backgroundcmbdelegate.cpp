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
#include "../data/activitydata.h"
#include "../data/layoutdata.h"
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

const int MARGIN = 2;

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

    QString value = index.model()->data(index, Qt::UserRole).toString();

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
    QString value = index.model()->data(index, Qt::UserRole).toString();

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
    model->setData(index, comboBox->currentText(), Qt::UserRole);
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

    QVariant background = index.data(Qt::UserRole);

    //! draw underlying background
    QStyledItemDelegate::paint(painter, myOptions, index);

    QList<IconData> icons;

    //! activities icons
    Data::ActivitiesMap allActivitiesData = index.data(Model::Layouts::ALLACTIVITIESDATAROLE).value<Data::ActivitiesMap>();

    bool isShared = index.data(Model::Layouts::LAYOUTISSHAREDROLE).toBool() && index.data(Model::Layouts::INMULTIPLELAYOUTSROLE).toBool();
    QStringList assignedIds =  isShared ? index.data(Model::Layouts::ASSIGNEDACTIVITIESFROMSHAREDROLE).toStringList() :
                                          index.data(Model::Layouts::ASSIGNEDACTIVITIESROLE).toStringList();

    int freeActivitiesPos = -1;

    for(int i=0; i<assignedIds.count(); ++i) {
        QString id = assignedIds[i];
        if (allActivitiesData.contains(id)) {
            IconData icon;
            icon.isBackground = false;
            icon.isFreeActivities = (id == Data::Layout::FREEACTIVITIESID);
            icon.name = allActivitiesData[id].icon;
            icons << icon;
            freeActivitiesPos = icons.count()-1;
        }
    }

    if (freeActivitiesPos>=0) {
        IconData freeActsData = icons.takeAt(freeActivitiesPos);
        icons.prepend(freeActsData);
    }

    //! background image
    if (background.isValid() && icons.count() == 0) {
        QString backgroundStr = background.toString();
        QString colorPath = backgroundStr.startsWith("/") ? backgroundStr : m_iconsPath + backgroundStr + "print.jpg";

        if (QFileInfo(colorPath).exists()) {
            IconData icon;
            icon.isBackground = true;
            icon.isFreeActivities = false;
            icon.name = colorPath;
            icons << icon;
        }
    }

    if (icons.count() > 0) {
        int localMargin = icons[0].isBackground ? qMin(option.rect.height()/4,MARGIN+5) : MARGIN-1;

        if (icons[0].isFreeActivities) {
            localMargin = 0;
        }

        int aY = option.rect.y() + localMargin;
        int thick = option.rect.height() - localMargin*2;

        int centerX = option.rect.x() + (option.rect.width() / 2);
        int step = thick;
        int total_icons_width = (thick-step) + icons.count() * step;

        if (total_icons_width > option.rect.width()){
            step = thick/2;
            total_icons_width = (thick-step) + icons.count() * step;
        }

        int startX = centerX - (total_icons_width/2);

        for (int i=0; i<icons.count(); ++i) {
            int tX = startX + (i * step);
            drawIcon(painter, option, QRect(tX, aY, thick, thick), icons[i]);
        }
    }
}

void BackgroundCmbBox::drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QRect &target, const IconData &icon) const
{
    bool active = Latte::isActive(option);
    bool selected = Latte::isSelected(option);
    bool focused = Latte::isFocused(option);

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (icon.isBackground) {
        QPixmap backImage(icon.name);
        backImage = backImage.copy(QRect(MARGIN, MARGIN, target.width(),target.height()));

        QPalette::ColorRole textColorRole = selected ? QPalette::HighlightedText : QPalette::Text;

        QBrush imageBrush(backImage);
        QPen pen; pen.setWidth(1);
        pen.setColor(option.palette.color(Latte::colorGroup(option), textColorRole));

        painter->setBrush(imageBrush);
        painter->setPen(pen);

        painter->drawEllipse(target);
    } else {
        QIcon::Mode mode = ((active && (selected || focused)) ? QIcon::Selected : QIcon::Normal);

        painter->drawPixmap(target, QIcon::fromTheme(icon.name).pixmap(target.height(), target.height(), mode));
    }

}

}
}
}
}

