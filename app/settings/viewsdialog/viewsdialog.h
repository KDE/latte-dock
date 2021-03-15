/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef VIEWSDIALOG_H
#define VIEWSDIALOG_H

// local
#include "../generic/genericdialog.h"
#include "../settingsdialog/settingsdialog.h"

// Qt
#include <QDialog>
#include <QObject>

namespace Ui {
class ViewsDialog;
}

namespace Latte {
namespace Settings {
namespace Controller {
class Layouts;
}
}
}

namespace Latte {
namespace Settings {
namespace Dialog {

class ViewsDialog : public GenericDialog
{
    Q_OBJECT

public:
    ViewsDialog(SettingsDialog *parent, Controller::Layouts *controller);
    ~ViewsDialog();

    Latte::Corona *corona() const;

    Ui::ViewsDialog *ui() const;
    Controller::Layouts *layoutsController() const;

protected:
    void accept() override;

private slots:
    void onOk();
    void onCancel();
    void onReset();

    void updateApplyButtonsState();

private:
    SettingsDialog *m_parentDlg{nullptr};
    Ui::ViewsDialog *m_ui;
    Controller::Layouts *m_layoutsController{nullptr};
};

}
}
}

#endif
