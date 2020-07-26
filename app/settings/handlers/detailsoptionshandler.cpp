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

#include "detailsoptionshandler.h"

// local
#include "ui_detailsdialog.h"
#include "detailshandler.h"
#include "../dialogs/detailsdialog.h"
#include "../widgets/patternwidget.h"
#include "../../layout/abstractlayout.h"

// Qt
#include <QColorDialog>
#include <QFileDialog>

namespace Latte {
namespace Settings {
namespace Handler {

DetailsOptionsHandler::DetailsOptionsHandler(Dialog::DetailsDialog *parentDialog, DetailsHandler *parentHandler)
    : Generic(parentDialog, parentHandler),
      m_parentDialog(parentDialog),
      m_ui(m_parentDialog->ui()),
      m_parentHandler(parentHandler)
{
    init();
}

DetailsOptionsHandler::~DetailsOptionsHandler()
{
}

void DetailsOptionsHandler::init()
{
    m_backButtonsGroup = new QButtonGroup(this);
    m_backButtonsGroup->addButton(m_ui->colorRadioBtn, Latte::Layout::ColorBackgroundStyle);
    m_backButtonsGroup->addButton(m_ui->backRadioBtn, Latte::Layout::PatternBackgroundStyle);
    m_backButtonsGroup->setExclusive(true);

    connect(m_backButtonsGroup, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {

        if (checked) {
            //m_layoutsController->setInMultipleMode(id == Latte::Types::MultipleLayouts);
        }
    });

    connect(m_ui->backgroundBtn, &QPushButton::pressed, this, &DetailsOptionsHandler::selectBackground);
    connect(m_ui->textColorBtn, &QPushButton::pressed, this, &DetailsOptionsHandler::selectTextColor);

    connect(m_ui->inMenuChk, &QCheckBox::stateChanged, this, [&]() {
        m_parentHandler->setIsShownInMenu(m_ui->inMenuChk->isChecked());
    });

    connect(m_ui->borderlessChk, &QCheckBox::stateChanged, this, [&]() {
        m_parentHandler->setHasDisabledBorders(m_ui->borderlessChk->isChecked());
    });

    connect(m_parentHandler, &DetailsHandler::currentLayoutChanged, this, &DetailsOptionsHandler::reload);

    reload();
}

void DetailsOptionsHandler::reload()
{
    loadLayout(m_parentHandler->currentData());
}

void DetailsOptionsHandler::loadLayout(const Data::Layout &data)
{
    if (data.backgroundStyle == Latte::Layout::ColorBackgroundStyle) {
        m_ui->colorRadioBtn->setChecked(true);
    } else {
        m_ui->backRadioBtn->setChecked(true);
    }

    m_ui->colorPatternWidget->setBackground(m_parentDialog->layoutsController()->colorPath(data.color));
    m_ui->backPatternWidget->setBackground(data.background);

    m_ui->colorPatternWidget->setTextColor(Layout::AbstractLayout::defaultTextColor(data.color));
    m_ui->backPatternWidget->setTextColor(data.textColor);

    m_ui->inMenuChk->setChecked(data.isShownInMenu);
    m_ui->borderlessChk->setChecked(data.hasDisabledBorders);
}

bool DetailsOptionsHandler::dataAreChanged() const
{
    return true;
}

bool DetailsOptionsHandler::inDefaultValues() const
{
    //nothing special
    return true;
}


void DetailsOptionsHandler::reset()
{
}

void DetailsOptionsHandler::resetDefaults()
{
    //do nothing
}

void DetailsOptionsHandler::save()
{
}

void DetailsOptionsHandler::selectBackground()
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
            m_parentHandler->setBackground(files[0]);
        }
    }
}

void DetailsOptionsHandler::selectTextColor()
{
    QColorDialog dialog(m_parentDialog);
    dialog.setCurrentColor(QColor(m_ui->backPatternWidget->textColor()));

    if (dialog.exec()) {
        qDebug() << "layout selected text color: " << dialog.selectedColor().name();
        m_parentHandler->setTextColor(dialog.selectedColor().name());
    }
}

}
}
}
