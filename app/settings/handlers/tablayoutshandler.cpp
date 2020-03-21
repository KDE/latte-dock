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

#include "tablayoutshandler.h"

//! local
#include "ui_settingsdialog.h"
#include "../settingsdialog.h"
#include "../universalsettings.h"
#include "../controllers/layoutscontroller.h"
#include "../../lattecorona.h"
#include "../../../liblatte2/types.h"

namespace Latte {
namespace Settings {
namespace Handler {


TabLayouts::TabLayouts(Latte::SettingsDialog *parent)
    : Generic(parent),
      m_parentDialog(parent),
      m_corona(m_parentDialog->corona()),
      m_ui(m_parentDialog->ui()),
      m_layoutsController(new Settings::Controller::Layouts(this))
{    
    initSettings();
    initUi();
}

void TabLayouts::initUi()
{

}

void TabLayouts::initSettings()
{

    updateUi();
}

void TabLayouts::updateUi()
{

}

Latte::Corona *TabLayouts::corona() const
{
    return m_corona;
}

Latte::SettingsDialog *TabLayouts::dialog() const
{
    return m_parentDialog;
}

Ui::SettingsDialog *TabLayouts::ui() const
{
    return m_ui;
}

bool TabLayouts::dataAreChanged() const
{
    return false;
}

bool TabLayouts::inDefaultValues() const
{
    return true;
}

void TabLayouts::reset()
{
    updateUi();
}

void TabLayouts::resetDefaults()
{

}

void TabLayouts::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const int &hideInterval)
{
    m_parentDialog->showInlineMessage(msg, type, hideInterval);
}

void TabLayouts::save()
{

}

}
}
}

