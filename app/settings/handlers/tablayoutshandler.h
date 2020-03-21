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

#ifndef SETTINGSTABLAYOUTSHANDLER_H
#define SETTINGSTABLAYOUTSHANDLER_H

//! local
#include "generichandler.h"

namespace Ui {
class SettingsDialog;
}

namespace Latte {
class Corona;
class SettingsDialog;

namespace Settings {
namespace Controller {
class Layouts;
}
}
}

namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class TabLayouts : public Generic
{
    Q_OBJECT
public:
    TabLayouts(Latte::SettingsDialog *parent);

    bool dataAreChanged() const override;
    bool inDefaultValues() const override;

    void reset() override;
    void resetDefaults() override;
    void save() override;

    Latte::Corona *corona() const;
    Latte::SettingsDialog *dialog() const;
    Ui::SettingsDialog *ui() const;

    void showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const int &hideInterval = 0) override;

private slots:
    void initUi();
    void initSettings();
    void updateUi();

private:
    Latte::SettingsDialog *m_parentDialog{nullptr};
    Ui::SettingsDialog *m_ui{nullptr};
    Latte::Corona *m_corona{nullptr};

    Settings::Controller::Layouts *m_layoutsController{nullptr};

};

}
}
}

#endif
