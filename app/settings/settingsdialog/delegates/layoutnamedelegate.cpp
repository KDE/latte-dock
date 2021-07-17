/*
    SPDX-FileCopyrightText: 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

    //! backround
    Latte::drawBackground(painter, option);

    painter->setRenderHint(QPainter::Antialiasing, true);

    //! Changes Indicator
    QRect remainedrect = Latte::remainedFromChangesIndicator(myOptions);
    if (isChanged) {
        Latte::drawChangesIndicator(painter, option);
    }

    myOptions.rect = remainedrect;

    if (hasErrors || hasWarnings) {
        remainedrect = Latte::remainedFromIcon(myOptions, Qt::AlignRight, -1, 2);
        if (hasErrors) {
            Latte::drawIcon(painter, myOptions, "data-error", Qt::AlignRight, -1, 2);
        } else if (hasWarnings) {
            Latte::drawIcon(painter, myOptions, "data-warning", Qt::AlignRight, -1, 2);
        }
        myOptions.rect = remainedrect;
    }

    if (isConsideredActive) {
        remainedrect = Latte::remainedFromIcon(myOptions, Qt::AlignRight, -1, 1);
        Latte::drawIcon(painter, myOptions, "favorite", Qt::AlignRight, -1, 1);
        myOptions.rect = remainedrect;
    }

    if (isLocked) {
        remainedrect = Latte::remainedFromIcon(myOptions, Qt::AlignRight, -1, 1);
        Latte::drawIcon(painter, myOptions, "object-locked", Qt::AlignRight, -1, 1);
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
