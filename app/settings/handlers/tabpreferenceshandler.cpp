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

#include "tabpreferenceshandler.h"

//! local
#include <coretypes.h>
#include "ui_settingsdialog.h"
#include "../universalsettings.h"
#include "../dialogs/settingsdialog.h"
#include "../../apptypes.h"
#include "../../lattecorona.h"
#include "../../plasma/extended/theme.h"


namespace Latte {
namespace Settings {
namespace Handler {


TabPreferences::TabPreferences(Latte::Settings::Dialog::SettingsDialog *parent)
    : Generic(parent),
      m_parentDialog(parent),
      m_corona(m_parentDialog->corona()),
      m_ui(m_parentDialog->ui())
{
    initSettings();
    initUi();
}

void TabPreferences::initUi()
{
    //! exclusive group
    m_mouseSensitivityButtons = new QButtonGroup(this);
    m_mouseSensitivityButtons->addButton(m_ui->lowSensitivityBtn, Latte::Settings::LowMouseSensitivity);
    m_mouseSensitivityButtons->addButton(m_ui->mediumSensitivityBtn, Latte::Settings::MediumMouseSensitivity);
    m_mouseSensitivityButtons->addButton(m_ui->highSensitivityBtn, Latte::Settings::HighMouseSensitivity);
    m_mouseSensitivityButtons->setExclusive(true);

    //! signals
    connect(m_mouseSensitivityButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {
        if (checked) {
            m_preferences.mouseSensitivity = static_cast<Latte::Settings::MouseSensitivity>(id);
            emit dataChanged();
        }
    });

    connect(m_ui->screenTrackerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ](int i) {
        m_preferences.screensDelay = m_ui->screenTrackerSpinBox->value();
        emit dataChanged();
    });

    connect(m_ui->outlineSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ](int i) {
        m_preferences.outlineWidth = m_ui->outlineSpinBox->value();
        emit dataChanged();
    });

    connect(m_ui->autostartChkBox, &QCheckBox::stateChanged, this, [&]() {
        m_preferences.autostart = m_ui->autostartChkBox->isChecked();
        emit dataChanged();
    });

    connect(m_ui->badges3DStyleChkBox, &QCheckBox::stateChanged, this, [&]() {
        m_preferences.badgeStyle3D = m_ui->badges3DStyleChkBox->isChecked();
        emit dataChanged();
    });

    connect(m_ui->screenTrackerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ](int i) {
        m_preferences.screensDelay = m_ui->screenTrackerSpinBox->value();
        emit dataChanged();
    });

    connect(m_ui->hiddenConfigsAreDeletedChk, &QCheckBox::stateChanged, this, [&]() {
        m_preferences.hiddenConfigurationWindowsAreDeleted = m_ui->hiddenConfigsAreDeletedChk->isChecked();
        emit dataChanged();
    });

    connect(m_ui->metaPressHoldChkBox, &QCheckBox::stateChanged, this, [&]() {
        m_preferences.metaHoldForBadges = m_ui->metaPressHoldChkBox->isChecked();
        emit dataChanged();
    });

    connect(m_ui->infoWindowChkBox, &QCheckBox::stateChanged, this, [&]() {
        m_preferences.layoutsInformationWindow = m_ui->infoWindowChkBox->isChecked();
        emit dataChanged();
    });

    connect(m_ui->noBordersForMaximizedChkBox, &QCheckBox::stateChanged, this, [&]() {
        m_preferences.borderlessMaximized = m_ui->noBordersForMaximizedChkBox->isChecked();
        emit dataChanged();
    });
}

void TabPreferences::initSettings()
{
    o_preferences.autostart = m_corona->universalSettings()->autostart();
    o_preferences.badgeStyle3D = m_corona->universalSettings()->badges3DStyle();
    o_preferences.layoutsInformationWindow = m_corona->universalSettings()->showInfoWindow();
    o_preferences.hiddenConfigurationWindowsAreDeleted = m_corona->universalSettings()->hiddenConfigurationWindowsAreDeleted();
    o_preferences.metaPressForAppLauncher = m_corona->universalSettings()->kwin_metaForwardedToLatte();
    o_preferences.metaHoldForBadges = m_corona->universalSettings()->metaPressAndHoldEnabled();
    o_preferences.borderlessMaximized = m_corona->universalSettings()->canDisableBorders();
    o_preferences.mouseSensitivity = m_corona->universalSettings()->sensitivity();
    o_preferences.screensDelay = m_corona->universalSettings()->screenTrackerInterval();
    o_preferences.outlineWidth = m_corona->themeExtended()->outlineWidth();

    m_preferences = o_preferences;

    updateUi();
}

void TabPreferences::updateUi()
{
    //! ui load
    m_ui->autostartChkBox->setChecked(m_preferences.autostart);
    m_ui->badges3DStyleChkBox->setChecked(m_preferences.badgeStyle3D);
    m_ui->infoWindowChkBox->setChecked(m_preferences.layoutsInformationWindow);
    m_ui->hiddenConfigsAreDeletedChk->setChecked(m_preferences.hiddenConfigurationWindowsAreDeleted);
    m_ui->metaPressChkBox->setChecked(m_preferences.metaPressForAppLauncher);
    m_ui->metaPressHoldChkBox->setChecked(m_preferences.metaHoldForBadges);
    m_ui->noBordersForMaximizedChkBox->setChecked(m_preferences.borderlessMaximized);
    m_ui->screenTrackerSpinBox->setValue(m_preferences.screensDelay);
    m_ui->outlineSpinBox->setValue(m_preferences.outlineWidth);

    if (m_preferences.mouseSensitivity == Settings::LowMouseSensitivity) {
        m_ui->lowSensitivityBtn->setChecked(true);
    } else if (m_preferences.mouseSensitivity == Settings::MediumMouseSensitivity) {
        m_ui->mediumSensitivityBtn->setChecked(true);
    } else if (m_preferences.mouseSensitivity == Settings::HighMouseSensitivity) {
        m_ui->highSensitivityBtn->setChecked(true);
    }

    emit dataChanged();
}

bool TabPreferences::dataAreChanged() const
{
    return o_preferences != m_preferences;
}

bool TabPreferences::inDefaultValues() const
{
    return m_preferences.inDefaultValues();
}

void TabPreferences::reset()
{
    m_preferences = o_preferences;
    updateUi();
}

void TabPreferences::resetDefaults()
{
    m_preferences.setToDefaults();
    updateUi();
}

void TabPreferences::save()
{
    m_corona->universalSettings()->setSensitivity(m_preferences.mouseSensitivity);
    m_corona->universalSettings()->setAutostart(m_preferences.autostart);
    m_corona->universalSettings()->setBadges3DStyle(m_preferences.badgeStyle3D);
    m_corona->universalSettings()->setHiddenConfigurationWindowsAreDeleted(m_preferences.hiddenConfigurationWindowsAreDeleted);
    m_corona->universalSettings()->kwin_forwardMetaToLatte(m_preferences.metaPressForAppLauncher);
    m_corona->universalSettings()->setMetaPressAndHoldEnabled(m_preferences.metaHoldForBadges);
    m_corona->universalSettings()->setShowInfoWindow(m_preferences.layoutsInformationWindow);
    m_corona->universalSettings()->setCanDisableBorders(m_preferences.borderlessMaximized);
    m_corona->universalSettings()->setScreenTrackerInterval(m_preferences.screensDelay);
    m_corona->themeExtended()->setOutlineWidth(m_preferences.outlineWidth);

    o_preferences = m_preferences;
    emit dataChanged();
}

}
}
}

