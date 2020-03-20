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

#ifndef SETTINGSPREFERENCESHANDLER_H
#define SETTINGSPREFERENCESHANDLER_H

//! local
#include "generichandler.h"
#include "../data/preferencesdata.h"

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
class SettingsDialog;
}

namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class Preferences : public Generic
{
    Q_OBJECT
public:
    Preferences(Latte::SettingsDialog *parent, Latte::Corona *corona);

    bool dataAreChanged() const;
    bool inDefaultValues() const;

    void reset();
    void resetDefaults();
    void save();

signals:
    void dataChanged();
    void borderlessMaximizedChanged();

private slots:
    void initUi();
    void updateUi();

    void loadSettings();

private:
    Latte::SettingsDialog *m_parentDialog{nullptr};
    Ui::SettingsDialog *m_ui{nullptr};
    Latte::Corona *m_corona{nullptr};

    QButtonGroup *m_mouseSensitivityButtons;

    //! current data
    Data::Preferences m_preferences;

    //! original data
    Data::Preferences o_preferences;
};

}
}
}

#endif
