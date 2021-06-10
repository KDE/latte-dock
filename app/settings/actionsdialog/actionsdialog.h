/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIONSDIALOG_H
#define ACTIONSDIALOG_H

// local
#include "../generic/genericdialog.h"

// Qt
#include <QDialog>

namespace Ui {
class ActionsDialog;
}

namespace Latte {
namespace Settings {
namespace Handler {
class ActionsHandler;
class TabPreferences;
}
}
}

namespace Latte {
namespace Settings {
namespace Dialog {

class ActionsDialog : public GenericDialog
{
    Q_OBJECT

public:
    ActionsDialog(QDialog *parent, Handler::TabPreferences *handler);
    ~ActionsDialog();

    Ui::ActionsDialog *ui() const;
    Handler::TabPreferences *preferencesHandler() const;

protected:
    void accept() override;

private slots:
    void onCancel();
   // void onDataChanged();
    void onReset();

private:
    void init();

private:
    Ui::ActionsDialog *m_ui;

    Handler::TabPreferences *m_preferencesHandler{nullptr};
    Handler::ActionsHandler *m_actionsHandler{nullptr};
};

}
}
}

#endif
