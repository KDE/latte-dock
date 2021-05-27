/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWSDIALOG_H
#define VIEWSDIALOG_H

// local
#include "../generic/genericdialog.h"
#include "../settingsdialog/settingsdialog.h"

// Qt
#include <QDialog>
#include <QObject>
#include <QPushButton>

namespace Ui {
class ViewsDialog;
}

namespace Latte {
namespace Settings {
namespace Controller {
class Layouts;
}
namespace Handler {
class ViewsHandler;
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
    void loadConfig();
    void saveConfig();

    void onOk();
    void onApply();
    void onCancel();
    void onReset();

    void updateApplyButtonsState();

private:
    SettingsDialog *m_parentDlg{nullptr};
    Ui::ViewsDialog *m_ui;
    Controller::Layouts *m_layoutsController{nullptr};

    QPushButton *m_applyNowBtn{nullptr};

    Handler::ViewsHandler *m_handler;   

    //! properties
    QSize m_windowSize;

    //! storage
    KConfigGroup m_storage;
};

}
}
}

#endif
