/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSTABPREFERENCESHANDLER_H
#define SETTINGSTABPREFERENCESHANDLER_H

//! local
#include "../generic/generichandler.h"
#include "../../data/preferencesdata.h"

//! Qt
#include <QAction>
#include <QObject>
#include <QButtonGroup>
#include <QPushButton>

namespace Ui {
class SettingsDialog;
}

namespace Latte {
class Corona;
namespace Settings {
namespace Dialog {
class SettingsDialog;
}
}
}

namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class TabPreferences : public Generic
{
    Q_OBJECT
public:
    TabPreferences(Latte::Settings::Dialog::SettingsDialog *parent);

    bool hasChangedData() const override;
    bool inDefaultValues() const override;

    QStringList contextMenuAlwaysActions() const;
    void setContextMenuAlwaysActions(const QStringList &actions);

    void reset() override;
    void resetDefaults() override;
    void save() override;

signals:
    void borderlessMaximizedChanged();
    void contextActionsChanged();

private slots:
    void initUi();
    void initSettings();
    void updateUi();

    void onActionsBtnPressed();

private:
    Latte::Settings::Dialog::SettingsDialog *m_parentDialog{nullptr};
    Ui::SettingsDialog *m_ui{nullptr};
    Latte::Corona *m_corona{nullptr};

    QButtonGroup *m_parabolicSpreadButtons;
    QButtonGroup *m_thicknessMarginInfluenceButtons;

    //! current data
    Data::Preferences m_preferences;

    //! original data
    Data::Preferences o_preferences;
};

}
}
}

#endif
