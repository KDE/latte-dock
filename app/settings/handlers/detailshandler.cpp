/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "detailshandler.h"

// local
#include "ui_detailsdialog.h"
#include "../controllers/layoutscontroller.h"
#include "../data/layoutstable.h"
#include "../delegates/colorcmbitemdelegate.h"
#include "../delegates/layoutcmbitemdelegate.h"
#include "../dialogs/detailsdialog.h"
#include "../models/colorsmodel.h"
#include "../models/layoutsmodel.h"
#include "../widgets/patternwidget.h"
#include "../../layout/abstractlayout.h"

// Qt
#include <QColorDialog>
#include <QFileDialog>
#include <QIcon>

// KDE
#include <KIconDialog>

namespace Latte {
namespace Settings {
namespace Handler {

DetailsHandler::DetailsHandler(Dialog::DetailsDialog *parentDialog)
    : Generic(parentDialog),
      m_parentDialog(parentDialog),
      m_ui(m_parentDialog->ui()),
      m_colorsModel(new Model::Colors(this, parentDialog->corona()))
{
    init();
}

DetailsHandler::~DetailsHandler()
{
}

void DetailsHandler::init()
{
    //! Layouts
    m_layoutsProxyModel = new QSortFilterProxyModel(this);
    m_layoutsProxyModel->setSourceModel(m_parentDialog->layoutsController()->baseModel());
    m_layoutsProxyModel->setSortRole(Model::Layouts::SORTINGROLE);
    m_layoutsProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_layoutsProxyModel->sort(Model::Layouts::NAMECOLUMN, Qt::AscendingOrder);

    m_ui->layoutsCmb->setModel(m_layoutsProxyModel);
    m_ui->layoutsCmb->setModelColumn(Model::Layouts::NAMECOLUMN);
    m_ui->layoutsCmb->setItemDelegate(new Settings::Layout::Delegate::LayoutCmbItemDelegate(this));

    //! Background Pattern
    m_backButtonsGroup = new QButtonGroup(this);
    m_backButtonsGroup->addButton(m_ui->colorRadioBtn, Latte::Layout::ColorBackgroundStyle);
    m_backButtonsGroup->addButton(m_ui->backRadioBtn, Latte::Layout::PatternBackgroundStyle);
    m_backButtonsGroup->setExclusive(true);

    m_ui->colorsCmb->setItemDelegate(new Details::Delegate::ColorCmbBoxItem(this));
    m_ui->colorsCmb->setModel(m_colorsModel);

    m_ui->patternClearBtn->setFixedHeight(m_ui->backgroundBtn->height()+2);

    connect(m_backButtonsGroup, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {

        if (checked) {
            setBackgroundStyle(static_cast<Latte::Layout::BackgroundStyle>(id));
        }
    });

    connect(m_ui->backgroundBtn, &QPushButton::pressed, this, &DetailsHandler::selectBackground);
    connect(m_ui->iconBtn, &QPushButton::pressed, this, &DetailsHandler::selectIcon);
    connect(m_ui->iconClearBtn, &QPushButton::pressed, this, &DetailsHandler::on_clearIcon);
    connect(m_ui->textColorBtn, &QPushButton::pressed, this, &DetailsHandler::selectTextColor);
    connect(m_ui->patternClearBtn, &QPushButton::pressed, this, &DetailsHandler::on_clearPattern);


    //! Options
    connect(m_ui->inMenuChk, &QCheckBox::stateChanged, this, [&]() {
        setIsShownInMenu(m_ui->inMenuChk->isChecked());
    });

    connect(m_ui->borderlessChk, &QCheckBox::stateChanged, this, [&]() {
        setHasDisabledBorders(m_ui->borderlessChk->isChecked());
    });

    connect(this, &DetailsHandler::currentLayoutChanged, this, &DetailsHandler::reload);

    reload();

    //! connect layout combobox after the selected layout has been loaded
    connect(m_ui->layoutsCmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DetailsHandler::on_currentLayoutIndexChanged);

    //! connect colors combobox after the selected layout has been loaded
    connect(m_ui->colorsCmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DetailsHandler::on_currentColorIndexChanged);


    //! pattern widgets
    connect(m_ui->backPatternWidget, &Widget::PatternWidget::mouseReleased, this, [&]() {
        setBackgroundStyle(Latte::Layout::PatternBackgroundStyle);
    });
    connect(m_ui->colorPatternWidget, &Widget::PatternWidget::mouseReleased, this, [&]() {
        setBackgroundStyle(Latte::Layout::ColorBackgroundStyle);
    });


    //! data were changed
    connect(this, &DetailsHandler::dataChanged, this, [&]() {
        loadLayout(c_data);
    });
}

void DetailsHandler::reload()
{
    o_data = m_parentDialog->layoutsController()->selectedLayoutCurrentData();
    c_data = o_data;

    m_ui->layoutsCmb->setCurrentText(o_data.name);

    loadLayout(c_data);
}

void DetailsHandler::loadLayout(const Data::Layout &data)
{
    if (data.icon.isEmpty()) {
        m_ui->iconBtn->setIcon(QIcon::fromTheme("add"));
        m_ui->iconClearBtn->setVisible(false);
    } else {
        m_ui->iconBtn->setIcon(QIcon::fromTheme(data.icon));
        m_ui->iconClearBtn->setVisible(true);
    }

    if (data.backgroundStyle == Latte::Layout::ColorBackgroundStyle) {
        m_ui->colorRadioBtn->setChecked(true);
        m_ui->backRadioBtn->setChecked(false);

        m_ui->colorsCmb->setVisible(true);
        m_ui->backgroundBtn->setVisible(false);
        m_ui->textColorBtn->setVisible(false);
        m_ui->patternClearBtn->setVisible(false);
    } else {
        m_ui->colorRadioBtn->setChecked(false);
        m_ui->backRadioBtn->setChecked(true);

        m_ui->colorsCmb->setVisible(false);
        m_ui->backgroundBtn->setVisible(true);
        m_ui->textColorBtn->setVisible(true);
        m_ui->patternClearBtn->setVisible(true);
    }

    m_ui->colorPatternWidget->setBackground(m_colorsModel->colorPath(data.color));
    m_ui->colorPatternWidget->setTextColor(Latte::Layout::AbstractLayout::defaultTextColor(data.color));

    m_ui->colorsCmb->setCurrentIndex(m_colorsModel->row(data.color));

    if (data.background.isEmpty()) {
        m_ui->backPatternWidget->setBackground(m_colorsModel->colorPath(Latte::Layout::AbstractLayout::defaultCustomBackground()));
        m_ui->backPatternWidget->setTextColor(Latte::Layout::AbstractLayout::defaultCustomTextColor());
    } else {
        m_ui->backPatternWidget->setBackground(data.background);
        m_ui->backPatternWidget->setTextColor(data.textColor);
    }

    if (!data.background.isEmpty() || !data.textColor.isEmpty()) {
        m_ui->patternClearBtn->setEnabled(true);
    } else {
        m_ui->patternClearBtn->setEnabled(false);
    }

    m_ui->inMenuChk->setChecked(data.isShownInMenu);
    m_ui->borderlessChk->setChecked(data.hasDisabledBorders);

    updateWindowTitle();
}

Data::Layout DetailsHandler::currentData() const
{
    return c_data;
}

bool DetailsHandler::dataAreChanged() const
{
    return o_data != c_data;
}

bool DetailsHandler::inDefaultValues() const
{
    //nothing special
    return true;
}


void DetailsHandler::reset()
{
    c_data = o_data;
    emit currentLayoutChanged();
}

void DetailsHandler::resetDefaults()
{
    //do nothing
}

void DetailsHandler::save()
{
}

void DetailsHandler::on_clearIcon()
{
    setIcon("");
}

void DetailsHandler::on_clearPattern()
{
    setBackground("");
    setTextColor("");
}

void DetailsHandler::on_currentColorIndexChanged(int row)
{
    QString selectedColor = m_ui->colorsCmb->itemData(row, Model::Colors::IDROLE).toString();
    setColor(selectedColor);
}

void DetailsHandler::on_currentLayoutIndexChanged(int row)
{
    QString layoutId = m_layoutsProxyModel->data(m_layoutsProxyModel->index(row, Model::Layouts::IDCOLUMN), Qt::UserRole).toString();
    m_parentDialog->layoutsController()->selectRow(layoutId);
    reload();

    emit currentLayoutChanged();
}

void DetailsHandler::setBackground(const QString &background)
{
    if (c_data.background == background) {
        return;
    }

    c_data.background = background;
    emit dataChanged();
}

void DetailsHandler::setColor(const QString &color)
{
    if (c_data.color == color) {
        return;
    }

    c_data.color = color;
    emit dataChanged();
}

void DetailsHandler::setIcon(const QString &icon)
{
    if (c_data.icon == icon) {
        return;
    }

    c_data.icon = icon;
    emit dataChanged();
}

void DetailsHandler::setTextColor(const QString &textColor)
{
    if (c_data.textColor == textColor) {
        return;
    }

    c_data.textColor = textColor;
    emit dataChanged();
}

void DetailsHandler::setIsShownInMenu(bool inMenu)
{
    if (c_data.isShownInMenu == inMenu) {
        return;
    }

    c_data.isShownInMenu = inMenu;
    emit dataChanged();
}

void DetailsHandler::setHasDisabledBorders(bool disabled)
{
    if (c_data.hasDisabledBorders == disabled) {
        return;
    }

    c_data.hasDisabledBorders = disabled;
    emit dataChanged();
}

void DetailsHandler::setBackgroundStyle(const Latte::Layout::BackgroundStyle &style)
{
    if (c_data.backgroundStyle == style) {
        return;
    }

    c_data.backgroundStyle = style;
    emit dataChanged();
}

void DetailsHandler::selectBackground()
{
    QStringList mimeTypeFilters;
    mimeTypeFilters << "image/jpeg" // will show "JPEG image (*.jpeg *.jpg)
                    << "image/png";  // will show "PNG image (*.png)"

    QFileDialog dialog(m_parentDialog);
    dialog.setMimeTypeFilters(mimeTypeFilters);

    QString background =  m_ui->backPatternWidget->background();

    if (background.startsWith("/") && QFileInfo(background).exists()) {
        dialog.setDirectory(QFileInfo(background).absolutePath());
        dialog.selectFile(background);
    }

    if (dialog.exec()) {
        QStringList files = dialog.selectedFiles();

        if (files.count() > 0) {
            setBackground(files[0]);
        }
    }
}

void DetailsHandler::selectIcon()
{
    QString icon = KIconDialog::getIcon();

    if (!icon.isEmpty()) {
        setIcon(icon);
    }
}

void DetailsHandler::selectTextColor()
{
    QColorDialog dialog(m_parentDialog);
    dialog.setCurrentColor(QColor(m_ui->backPatternWidget->textColor()));

    if (dialog.exec()) {
        qDebug() << "layout selected text color: " << dialog.selectedColor().name();
        setTextColor(dialog.selectedColor().name());
    }
}

void DetailsHandler::updateWindowTitle()
{
    m_parentDialog->setWindowTitle(m_ui->layoutsCmb->currentText());
}

}
}
}
