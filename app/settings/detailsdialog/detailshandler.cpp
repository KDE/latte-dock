/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "detailshandler.h"

// local
#include "ui_detailsdialog.h"
#include "colorsmodel.h"
#include "detailsdialog.h"
#include "patternwidget.h"
#include "schemesmodel.h"
#include "delegates/colorcmbitemdelegate.h"
#include "delegates/schemecmbitemdelegate.h"
#include "../settingsdialog/layoutscontroller.h"
#include "../settingsdialog/layoutsmodel.h"
#include "../settingsdialog/delegates/layoutcmbitemdelegate.h"
#include "../../data/layoutstable.h"
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

DetailsHandler::DetailsHandler(Dialog::DetailsDialog *dialog)
    : Generic(dialog),
      m_dialog(dialog),
      m_ui(m_dialog->ui()),
      m_colorsModel(new Model::Colors(this, dialog->corona())),
      m_schemesModel(new Model::Schemes(this))
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
    m_layoutsProxyModel->setSourceModel(m_dialog->layoutsController()->baseModel());
    m_layoutsProxyModel->setSortRole(Model::Layouts::SORTINGROLE);
    m_layoutsProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_layoutsProxyModel->sort(Model::Layouts::NAMECOLUMN, Qt::AscendingOrder);

    m_ui->layoutsCmb->setModel(m_layoutsProxyModel);
    m_ui->layoutsCmb->setModelColumn(Model::Layouts::NAMECOLUMN);
    m_ui->layoutsCmb->setItemDelegate(new Settings::Layout::Delegate::LayoutCmbItemDelegate(this));

    //! Schemes
    m_ui->customSchemeCmb->setModel(m_schemesModel);
    m_ui->customSchemeCmb->setItemDelegate(new Settings::Details::Delegate::SchemeCmbItemDelegate(this));

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
    connect(m_ui->iconClearBtn, &QPushButton::pressed, this, &DetailsHandler::clearIcon);
    connect(m_ui->textColorBtn, &QPushButton::pressed, this, &DetailsHandler::selectTextColor);
    connect(m_ui->patternClearBtn, &QPushButton::pressed, this, &DetailsHandler::clearPattern);


    //! Options
    connect(m_ui->popUpMarginSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [&](int i) {
        setPopUpMargin(i);
    });

    connect(m_ui->inMenuChk, &QCheckBox::stateChanged, this, [&]() {
        setIsShownInMenu(m_ui->inMenuChk->isChecked());
    });

    connect(m_ui->borderlessChk, &QCheckBox::stateChanged, this, [&]() {
        setHasDisabledBorders(m_ui->borderlessChk->isChecked());
    });

    connect(this, &DetailsHandler::currentLayoutChanged, this, &DetailsHandler::reload);

    reload();
    m_lastConfirmedLayoutIndex = m_ui->colorsCmb->currentIndex();

    //! connect layout combobox after the selected layout has been loaded
    connect(m_ui->layoutsCmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DetailsHandler::onCurrentLayoutIndexChanged);

    //! connect colors combobox after the selected layout has been loaded
    connect(m_ui->colorsCmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DetailsHandler::onCurrentColorIndexChanged);

    //! connect custom scheme combobox after the selected layout has been loaded
    connect(m_ui->customSchemeCmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DetailsHandler::onCurrentSchemeIndexChanged);

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
    o_data = m_dialog->layoutsController()->selectedLayoutCurrentData();
    c_data = o_data;

    Latte::Data::LayoutIcon icon = m_dialog->layoutsController()->selectedLayoutIcon();

    m_ui->layoutsCmb->setCurrentText(o_data.name);
    m_ui->layoutsCmb->setLayoutIcon(icon);

    loadLayout(c_data);
}

void DetailsHandler::loadLayout(const Latte::Data::Layout &data)
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

    int schind = m_schemesModel->row(data.schemeFile);
    m_ui->customSchemeCmb->setCurrentIndex(schind);
    updateCustomSchemeCmb(schind);

    m_ui->colorPatternWidget->setBackground(m_colorsModel->colorPath(data.color));
    m_ui->colorPatternWidget->setTextColor(Latte::Layout::AbstractLayout::defaultTextColor(data.color));

    m_ui->colorsCmb->setCurrentIndex(m_colorsModel->row(data.color));

    if (data.background.isEmpty()) {
        m_ui->backPatternWidget->setBackground(m_colorsModel->colorPath(Latte::Layout::AbstractLayout::defaultCustomBackground()));
    } else {
        m_ui->backPatternWidget->setBackground(data.background);
    }

    if (data.background.isEmpty() && data.textColor.isEmpty()) {
        m_ui->backPatternWidget->setTextColor(Latte::Layout::AbstractLayout::defaultCustomTextColor());
    } else {
        m_ui->backPatternWidget->setTextColor(data.textColor);
    }

    if (!data.background.isEmpty() || !data.textColor.isEmpty()) {
        m_ui->patternClearBtn->setEnabled(true);
    } else {
        m_ui->patternClearBtn->setEnabled(false);
    }

    m_ui->popUpMarginSpinBox->setValue(data.popUpMargin);

    m_ui->inMenuChk->setChecked(data.isShownInMenu);
    m_ui->borderlessChk->setChecked(data.hasDisabledBorders);

    updateWindowTitle();
}

Latte::Data::Layout DetailsHandler::currentData() const
{
    return c_data;
}

bool DetailsHandler::hasChangedData() const
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
    m_dialog->layoutsController()->setLayoutProperties(currentData());
}

void DetailsHandler::clearIcon()
{
    setIcon("");
}

void DetailsHandler::clearPattern()
{
    setBackground("");
    setTextColor("");
}

void DetailsHandler::onCurrentColorIndexChanged(int row)
{
    QString selectedColor = m_ui->colorsCmb->itemData(row, Model::Colors::IDROLE).toString();
    setColor(selectedColor);
}

void DetailsHandler::onCurrentLayoutIndexChanged(int row)
{
    bool switchtonewlayout{false};

    if (m_lastConfirmedLayoutIndex != row) {
        if (hasChangedData()) { //new layout was chosen but there are changes
            KMessageBox::ButtonCode result = saveChangesConfirmation();

            if (result == KMessageBox::Yes) {
                switchtonewlayout = true;
                m_lastConfirmedLayoutIndex = row;
                save();
            } else if (result == KMessageBox::No) {
                switchtonewlayout = true;
                m_lastConfirmedLayoutIndex = row;
            } else if (result == KMessageBox::Cancel) {
                //do nothing
            }
        } else { //new layout was chosen and there are no changes
            switchtonewlayout = true;
            m_lastConfirmedLayoutIndex = row;
        }
    }

    if (switchtonewlayout) {
        QString layoutId = m_layoutsProxyModel->data(m_layoutsProxyModel->index(row, Model::Layouts::IDCOLUMN), Qt::UserRole).toString();
        m_dialog->layoutsController()->selectRow(layoutId);
        reload();
        emit currentLayoutChanged();
    } else {
        //! reset combobox index
        m_ui->layoutsCmb->setCurrentText(c_data.name);
    }
}

void DetailsHandler::updateCustomSchemeCmb(const int &row)
{
    int scmind = row;
    m_ui->customSchemeCmb->setCurrentText(m_ui->customSchemeCmb->itemData(scmind, Qt::DisplayRole).toString());
    m_ui->customSchemeCmb->setTextColor(m_ui->customSchemeCmb->itemData(scmind, Model::Schemes::TEXTCOLORROLE).value<QColor>());
    m_ui->customSchemeCmb->setBackgroundColor(m_ui->customSchemeCmb->itemData(scmind, Model::Schemes::BACKGROUNDCOLORROLE).value<QColor>());
}

void DetailsHandler::onCurrentSchemeIndexChanged(int row)
{
    updateCustomSchemeCmb(row);
    QString selectedScheme = m_ui->customSchemeCmb->itemData(row, Model::Schemes::IDROLE).toString();
    setCustomSchemeFile(selectedScheme);
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

void DetailsHandler::setCustomSchemeFile(const QString &file)
{
    if (c_data.schemeFile == file) {
        return;
    }

    c_data.schemeFile = file;
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

void DetailsHandler::setPopUpMargin(const int &margin)
{
    if (c_data.popUpMargin == margin) {
        return;
    }

    c_data.popUpMargin = margin;
    emit dataChanged();
}

void DetailsHandler::selectBackground()
{
    QStringList mimeTypeFilters;
    mimeTypeFilters << "image/jpeg" // will show "JPEG image (*.jpeg *.jpg)
                    << "image/png";  // will show "PNG image (*.png)"

    QFileDialog dialog(m_dialog);
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
    QColorDialog dialog(m_dialog);
    dialog.setCurrentColor(QColor(m_ui->backPatternWidget->textColor()));

    if (dialog.exec()) {
        qDebug() << "layout selected text color: " << dialog.selectedColor().name();
        setTextColor(dialog.selectedColor().name());
    }
}

void DetailsHandler::updateWindowTitle()
{
    m_dialog->setWindowTitle(i18nc("<layout name> Details","%1 Details", m_ui->layoutsCmb->currentText()));
}

KMessageBox::ButtonCode DetailsHandler::saveChangesConfirmation()
{
    if (hasChangedData()) {
        QString layoutName = c_data.name;
        QString saveChangesText = i18n("The settings of <b>%1</b> layout have changed.<br/>Do you want to apply the changes or discard them?", layoutName);

        return m_dialog->saveChangesConfirmation(saveChangesText);
    }

    return KMessageBox::Cancel;
}

}
}
}
