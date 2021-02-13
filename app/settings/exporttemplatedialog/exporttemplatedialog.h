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

#ifndef EXPORTTEMPLATEDIALOG_H
#define EXPORTTEMPLATEDIALOG_H

// local
#include "../generic/genericdialog.h"
#include "../settingsdialog/settingsdialog.h"

// Qt
#include <QDialog>
#include <QObject>
#include <QPushButton>
#include <QWindow>

namespace Ui {
class ExportTemplateDialog;
}

namespace Latte {
class View;
namespace Settings {
namespace Controller {
class Layouts;
}
namespace Handler {
class ExportTemplateHandler;
}
}
}


namespace Latte {
namespace Settings {
namespace Dialog {

class ExportTemplateDialog : public GenericDialog
{
    Q_OBJECT

public:
    ExportTemplateDialog(SettingsDialog *parent, const QString &layoutName, const QString &layoutId);
    ExportTemplateDialog(Latte::View *view);
    ~ExportTemplateDialog();

    Ui::ExportTemplateDialog *ui() const;
    Latte::Corona *corona() const;

    QPushButton *exportButton() const;

protected:
    void accept() override;

private slots:
    void onCancel();
    void onDataChanged();
    void onReset();

    void initButtons();
    void initExportButton(const QString &tooltip);

private:
    void init();

private:
    bool m_isExportingLayout{false};
    bool m_isExportingView{false};

    QPushButton *m_exportButton{nullptr};

    Latte::Corona *m_corona{nullptr};

    Ui::ExportTemplateDialog *m_ui;
    Controller::Layouts *m_layoutsController{nullptr};

    Handler::ExportTemplateHandler *m_handler;
};

}
}
}

#endif
