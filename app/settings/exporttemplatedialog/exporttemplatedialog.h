/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXPORTTEMPLATEDIALOG_H
#define EXPORTTEMPLATEDIALOG_H

// local
#include "../generic/genericdialog.h"
#include "../viewsdialog/viewsdialog.h"
#include "../settingsdialog/settingsdialog.h"
#include "../../data/layoutdata.h"
#include "../../data/viewdata.h"

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
    ExportTemplateDialog(QDialog *parent);
    ExportTemplateDialog(SettingsDialog *parent, const Data::Layout &layout);
    ExportTemplateDialog(ViewsDialog *parent, const Data::View &view);
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
    void onExportSucceeded();
    void onReset();

    void initButtons();
    void initExportButton(const QString &tooltip);
    void initSignals();

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
