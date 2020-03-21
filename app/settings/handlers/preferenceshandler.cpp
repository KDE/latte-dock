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

#include "preferenceshandler.h"

//! local
#include "ui_settingsdialog.h"
#include "../settingsdialog.h"
#include "../universalsettings.h"
#include "../../lattecorona.h"
#include "../../plasma/extended/theme.h"
#include "../../../liblatte2/types.h"

namespace Latte {
namespace Settings {
namespace Handler {


Preferences::Preferences(Latte::SettingsDialog *parent)
    : Generic(parent),
      m_parentDialog(parent),
      m_corona(m_parentDialog->corona()),
      m_ui(m_parentDialog->ui())
{
    initSettings();
    initUi();
}

void Preferences::initUi()
{
    //! exclusive group
    m_mouseSensitivityButtons = new QButtonGroup(this);
    m_mouseSensitivityButtons->addButton(m_ui->lowSensitivityBtn, Latte::Types::LowSensitivity);
    m_mouseSensitivityButtons->addButton(m_ui->mediumSensitivityBtn, Latte::Types::MediumSensitivity);
    m_mouseSensitivityButtons->addButton(m_ui->highSensitivityBtn, Latte::Types::HighSensitivity);
    m_mouseSensitivityButtons->setExclusive(true);

    //! signals
    connect(m_mouseSensitivityButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {
        if (checked) {
            m_preferences.mouseSensitivity = static_cast<Latte::Types::MouseSensitivity>(id);
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

    connect(m_ui->metaPressChkBox, &QCheckBox::stateChanged, this, [&]() {
        m_preferences.metaPressForAppLauncher = m_ui->metaPressChkBox->isChecked();
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

void Preferences::initSettings()
{
    o_preferences.autostart = m_corona->universalSettings()->autostart();
    o_preferences.badgeStyle3D = m_corona->universalSettings()->badges3DStyle();
    o_preferences.layoutsInformationWindow = m_corona->universalSettings()->showInfoWindow();
    o_preferences.metaPressForAppLauncher = m_corona->universalSettings()->kwin_metaForwardedToLatte();
    o_preferences.metaHoldForBadges = m_corona->universalSettings()->metaPressAndHoldEnabled();
    o_preferences.borderlessMaximized = m_corona->universalSettings()->canDisableBorders();
    o_preferences.mouseSensitivity = m_corona->universalSettings()->mouseSensitivity();
    o_preferences.screensDelay = m_corona->universalSettings()->screenTrackerInterval();
    o_preferences.outlineWidth = m_corona->themeExtended()->outlineWidth();

    m_preferences = o_preferences;

    updateUi();
}

void Preferences::updateUi()
{
    //! ui load
    m_ui->autostartChkBox->setChecked(m_preferences.autostart);
    m_ui->badges3DStyleChkBox->setChecked(m_preferences.badgeStyle3D);
    m_ui->infoWindowChkBox->setChecked(m_preferences.layoutsInformationWindow);
    m_ui->metaPressChkBox->setChecked(m_preferences.metaPressForAppLauncher);
    m_ui->metaPressHoldChkBox->setChecked(m_preferences.metaHoldForBadges);
    m_ui->noBordersForMaximizedChkBox->setChecked(m_preferences.borderlessMaximized);
    m_ui->screenTrackerSpinBox->setValue(m_preferences.screensDelay);
    m_ui->outlineSpinBox->setValue(m_preferences.outlineWidth);

    if (m_preferences.mouseSensitivity == Types::LowSensitivity) {
        m_ui->lowSensitivityBtn->setChecked(true);
    } else if (m_preferences.mouseSensitivity == Types::MediumSensitivity) {
        m_ui->mediumSensitivityBtn->setChecked(true);
    } else if (m_preferences.mouseSensitivity == Types::HighSensitivity) {
        m_ui->highSensitivityBtn->setChecked(true);
    }

    emit dataChanged();
}

bool Preferences::dataAreChanged() const
{
    return o_preferences != m_preferences;
}

bool Preferences::inDefaultValues() const
{
    return m_preferences.inDefaultValues();
}

void Preferences::reset()
{
    m_preferences = o_preferences;
    updateUi();
}

void Preferences::resetDefaults()
{
    m_preferences.setToDefaults();
    updateUi();
}

void Preferences::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const int &hideInterval)
{
    m_parentDialog->showInlineMessage(msg, type, hideInterval);
}

void Preferences::save()
{
    m_corona->universalSettings()->setMouseSensitivity(m_preferences.mouseSensitivity);
    m_corona->universalSettings()->setAutostart(m_preferences.autostart);
    m_corona->universalSettings()->setBadges3DStyle(m_preferences.badgeStyle3D);
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

