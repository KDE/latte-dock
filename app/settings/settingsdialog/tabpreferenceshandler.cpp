/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabpreferenceshandler.h"

//! local
#include <coretypes.h>
#include "ui_settingsdialog.h"
#include "settingsdialog.h"
#include "../universalsettings.h"
#include "../actionsdialog/actionsdialog.h"
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
    //! exclusive groups
    m_parabolicSpreadButtons = new QButtonGroup(this);
    m_parabolicSpreadButtons->addButton(m_ui->smallParabolicBtn, Data::Preferences::PARABOLICSPREAD);
    m_parabolicSpreadButtons->addButton(m_ui->mediumParabolicBtn, 5);
    m_parabolicSpreadButtons->addButton(m_ui->largeParabolicBtn, 7);
    m_parabolicSpreadButtons->setExclusive(true);

    m_thicknessMarginInfluenceButtons = new QButtonGroup(this);
    m_thicknessMarginInfluenceButtons->addButton(m_ui->noMarginInfluenceBtn, 0); // 0%
    m_thicknessMarginInfluenceButtons->addButton(m_ui->halfMarginInfluenceBtn, 50); // 50%
    m_thicknessMarginInfluenceButtons->addButton(m_ui->fullMarginInfluenceBtn, 100); // 100%
    m_thicknessMarginInfluenceButtons->setExclusive(true);

    m_ui->noMarginInfluenceBtn->setText(i18nc("number in percentage, e.g. 85%","%1%").arg(0));
    m_ui->halfMarginInfluenceBtn->setText(i18nc("number in percentage, e.g. 85%","%1%").arg(50));
    m_ui->fullMarginInfluenceBtn->setText(i18nc("number in percentage, e.g. 85%","%1%").arg(100));

    //! Buttons
    connect(m_ui->contextMenuActionsBtn, &QPushButton::clicked, this, &TabPreferences::onActionsBtnPressed);

    //! signals
    connect(m_parabolicSpreadButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {
        if (checked) {
            m_preferences.parabolicSpread = id;
            emit dataChanged();
        }
    });

    connect(m_thicknessMarginInfluenceButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {
        if (checked) {
            m_preferences.thicknessMarginInfluence = (id / 100.0f);
            emit dataChanged();
        }
    });

    connect(m_ui->screenTrackerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ](int i) {
        m_preferences.screensDelay = m_ui->screenTrackerSpinBox->value();
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

    connect(m_ui->broadcastGeomChkBox, &QCheckBox::stateChanged, this, [&]() {
        m_preferences.isAvailableGeometryBroadcastedToPlasma = m_ui->broadcastGeomChkBox->isChecked();
        emit dataChanged();
    });

    connect(m_ui->screenTrackerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ](int i) {
        m_preferences.screensDelay = m_ui->screenTrackerSpinBox->value();
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

    connect(this, &TabPreferences::contextActionsChanged, this, &TabPreferences::dataChanged);
}

void TabPreferences::initSettings()
{
    o_preferences.autostart = m_corona->universalSettings()->autostart();
    o_preferences.badgeStyle3D = m_corona->universalSettings()->badges3DStyle();
    o_preferences.contextMenuAlwaysActions = m_corona->universalSettings()->contextMenuActionsAlwaysShown();
    o_preferences.isAvailableGeometryBroadcastedToPlasma = m_corona->universalSettings()->isAvailableGeometryBroadcastedToPlasma();
    o_preferences.layoutsInformationWindow = m_corona->universalSettings()->showInfoWindow();
    o_preferences.metaPressForAppLauncher = m_corona->universalSettings()->kwin_metaForwardedToLatte();
    o_preferences.metaHoldForBadges = m_corona->universalSettings()->metaPressAndHoldEnabled();
    o_preferences.borderlessMaximized = m_corona->universalSettings()->canDisableBorders();
    o_preferences.parabolicSpread = m_corona->universalSettings()->parabolicSpread();
    o_preferences.thicknessMarginInfluence = m_corona->universalSettings()->thicknessMarginInfluence();
    o_preferences.screensDelay = m_corona->universalSettings()->screenTrackerInterval();

    m_preferences = o_preferences;

    updateUi();
}

QStringList TabPreferences::contextMenuAlwaysActions() const
{
    return m_preferences.contextMenuAlwaysActions;
}

void TabPreferences::setContextMenuAlwaysActions(const QStringList &actions)
{
    if (m_preferences.contextMenuAlwaysActions == actions) {
        return;
    }

    m_preferences.contextMenuAlwaysActions = actions;
    emit contextActionsChanged();
}

void TabPreferences::updateUi()
{
    //! ui load
    m_ui->autostartChkBox->setChecked(m_preferences.autostart);
    m_ui->badges3DStyleChkBox->setChecked(m_preferences.badgeStyle3D);
    m_ui->infoWindowChkBox->setChecked(m_preferences.layoutsInformationWindow);
    m_ui->broadcastGeomChkBox->setChecked(m_preferences.isAvailableGeometryBroadcastedToPlasma);
    m_ui->metaPressChkBox->setChecked(m_preferences.metaPressForAppLauncher);
    m_ui->metaPressHoldChkBox->setChecked(m_preferences.metaHoldForBadges);
    m_ui->noBordersForMaximizedChkBox->setChecked(m_preferences.borderlessMaximized);
    m_ui->screenTrackerSpinBox->setValue(m_preferences.screensDelay);

    if (m_preferences.parabolicSpread == Data::Preferences::PARABOLICSPREAD) {
        m_ui->smallParabolicBtn->setChecked(true);
    } else if (m_preferences.parabolicSpread == 5) {
        m_ui->mediumParabolicBtn->setChecked(true);
    } else if (m_preferences.parabolicSpread == 7) {
        m_ui->largeParabolicBtn->setChecked(true);
    }

    if (m_preferences.thicknessMarginInfluence == 0.0f) {
        m_ui->noMarginInfluenceBtn->setChecked(true);
    } else if (m_preferences.thicknessMarginInfluence == 0.5f) {
        m_ui->halfMarginInfluenceBtn->setChecked(true);
    } else if (m_preferences.thicknessMarginInfluence == Data::Preferences::THICKNESSMARGININFLUENCE) {
        m_ui->fullMarginInfluenceBtn->setChecked(true);
    }

    emit dataChanged();
}

bool TabPreferences::hasChangedData() const
{
    return o_preferences != m_preferences;
}

bool TabPreferences::inDefaultValues() const
{
    return m_preferences.inDefaultValues();
}

void TabPreferences::onActionsBtnPressed()
{
    auto viewsDlg = new Settings::Dialog::ActionsDialog(m_parentDialog, this);
    viewsDlg->exec();
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
    m_corona->universalSettings()->setAutostart(m_preferences.autostart);
    m_corona->universalSettings()->setBadges3DStyle(m_preferences.badgeStyle3D);
    m_corona->universalSettings()->setContextMenuActionsAlwaysShown(m_preferences.contextMenuAlwaysActions);
    m_corona->universalSettings()->kwin_forwardMetaToLatte(m_preferences.metaPressForAppLauncher);
    m_corona->universalSettings()->setIsAvailableGeometryBroadcastedToPlasma(m_preferences.isAvailableGeometryBroadcastedToPlasma);
    m_corona->universalSettings()->setMetaPressAndHoldEnabled(m_preferences.metaHoldForBadges);
    m_corona->universalSettings()->setShowInfoWindow(m_preferences.layoutsInformationWindow);
    m_corona->universalSettings()->setCanDisableBorders(m_preferences.borderlessMaximized);
    m_corona->universalSettings()->setParabolicSpread(m_preferences.parabolicSpread);
    m_corona->universalSettings()->setThicknessMarginInfluence(m_preferences.thicknessMarginInfluence);
    m_corona->universalSettings()->setScreenTrackerInterval(m_preferences.screensDelay);

    o_preferences = m_preferences;
    emit dataChanged();
}

}
}
}

