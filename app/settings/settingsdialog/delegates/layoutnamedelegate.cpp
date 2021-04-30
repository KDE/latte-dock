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

#include "layoutnamedelegate.h"

// local
#include "../layoutsmodel.h"
#include "../../generic/generictools.h"

// Qt
#include <QApplication>
#include <QBitmap>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItemModel>

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

const int INDICATORCHANGESLENGTH = 6;
const int INDICATORCHANGESMARGIN = 2;

LayoutName::LayoutName(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *LayoutName::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QLineEdit *editor = new QLineEdit(parent);
    return editor;
}

void LayoutName::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEditor = qobject_cast<QLineEdit *>(editor);

    if (lineEditor) {
        QString name = index.data(Qt::UserRole).toString();
        lineEditor->setText(name);
    }
}

void LayoutName::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEditor = qobject_cast<QLineEdit *>(editor);

    if (lineEditor) {
        model->setData(index, lineEditor->text(), Qt::UserRole);
    }
}

void LayoutName::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool inMultiple = index.data(Model::Layouts::INMULTIPLELAYOUTSROLE).toBool();

    bool isLocked = index.data(Model::Layouts::ISLOCKEDROLE).toBool();
    bool isActive = index.data(Model::Layouts::ISACTIVEROLE).toBool();
    bool isConsideredActive = index.data(Model::Layouts::ISCONSIDEREDACTIVEROLE).toBool();

    bool isNewLayout = index.data(Model::Layouts::ISNEWLAYOUTROLE).toBool();
    bool hasChanges = index.data(Model::Layouts::LAYOUTHASCHANGESROLE).toBool();
    bool hasErrors = index.data(Model::Layouts::ERRORSROLE).toBool();
    bool hasWarnings = index.data(Model::Layouts::WARNINGSROLE).toBool();

    QString name = index.data(Qt::UserRole).toString();

    bool isChanged = (isNewLayout || hasChanges);

    QStyleOptionViewItem myOptions = option;
    myOptions.text = name;

    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);
    myOptions.displayAlignment = static_cast<Qt::Alignment>(index.model()->data(index, Qt::TextAlignmentRole).toInt());;

    painter->setRenderHint(QPainter::Antialiasing, true);

    //! Changes Indicator
    QRect remainedrect = Latte::remainedFromChangesIndicator(myOptions);
    Latte::drawChangesIndicatorBackground(painter, myOptions);

    if (isChanged) {
        Latte::drawChangesIndicator(painter, option);
    }

    myOptions.rect = remainedrect;

    if (hasErrors || hasWarnings) {
        remainedrect = Latte::remainedFromIcon(myOptions, Qt::AlignRight);
        Latte::drawIconBackground(painter, myOptions, Qt::AlignRight);
        if (hasErrors) {
            Latte::drawIcon(painter, myOptions, "data-error", Qt::AlignRight);
        } else if (hasWarnings) {
            Latte::drawIcon(painter, myOptions, "data-warning", Qt::AlignRight);
        }
        myOptions.rect = remainedrect;
    }

    if (isConsideredActive) {
        remainedrect = Latte::remainedFromIcon(myOptions, Qt::AlignRight);
        Latte::drawIconBackground(painter, myOptions, Qt::AlignRight);
        Latte::drawIcon(painter, myOptions, "favorites", Qt::AlignRight);
        myOptions.rect = remainedrect;
    }

    if (isLocked) {
        remainedrect = Latte::remainedFromIcon(myOptions, Qt::AlignRight);
        Latte::drawIconBackground(painter, myOptions, Qt::AlignRight);
        Latte::drawIcon(painter, myOptions, "object-locked", Qt::AlignRight);
        myOptions.rect = remainedrect;
    }

    if (isActive) {
        myOptions.text = "<b>" + myOptions.text + "</b>";
    }

    if (isChanged) {
        myOptions.text = "<i>" + myOptions.text + "</i>";
    }

    Latte::drawFormattedText(painter, myOptions);
}

}
}
}
}
