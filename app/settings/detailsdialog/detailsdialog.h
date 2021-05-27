/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DETAILSDIALOG_H
#define DETAILSDIALOG_H

// local
#include "../generic/genericdialog.h"
#include "../settingsdialog/settingsdialog.h"

// Qt
#include <QDialog>
#include <QObject>

namespace Ui {
class DetailsDialog;
}

namespace Latte {
namespace Settings {
namespace Controller {
class Layouts;
}
namespace Handler {
class DetailsHandler;
}
}
}

namespace Latte {
namespace Settings {
namespace Dialog {

class DetailsDialog : public GenericDialog
{
    Q_OBJECT

public:
    DetailsDialog(SettingsDialog *parent, Controller::Layouts *controller);
    ~DetailsDialog();

    Latte::Corona *corona() const;

    Ui::DetailsDialog *ui() const;
    Controller::Layouts *layoutsController() const;

protected:
    void accept() override;

private slots:
    void loadConfig();
    void saveConfig();

    void onOk();
    void onCancel();
    void onReset();

    void updateApplyButtonsState();

private:
    SettingsDialog *m_parentDlg{nullptr};
    Ui::DetailsDialog *m_ui;
    Controller::Layouts *m_layoutsController{nullptr};

    Handler::DetailsHandler *m_handler;

    //! properties
    QSize m_windowSize;

    //! storage
    KConfigGroup m_storage;
};

}
}
}

#endif
