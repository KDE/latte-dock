/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIONSHANDLER_H
#define ACTIONSHANDLER_H

// local
#include "actionlistwidgetitem.h"
#include "../generic/generichandler.h"
#include "../../data/generictable.h"

// Qt
#include <QHash>
#include <QObject>

namespace Ui {
class ActionsDialog;
}

namespace Latte{
namespace Settings{
namespace Dialog{
class ActionsDialog;
}
}
}


namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class ActionsHandler : public Generic
{
    Q_OBJECT
public:
    ActionsHandler(Dialog::ActionsDialog *dialog);
    ~ActionsHandler();

    bool hasChangedData() const override;
    bool inDefaultValues() const override;

    QStringList currentAlwaysData() const;

public slots:
    void reset() override;
    void resetDefaults() override;
    void save() override;
    void updateButtonEnablement();

    void onCancel();

private:
    void init();
    void initItems();

    void loadItems(const QStringList &alwaysActions);

    Data::GenericTable<Data::Generic> table(const QStringList &ids);

private:
    QStringList o_alwaysActions;

    QHash<QString, Settings::ActionsDialog::ActionListWidgetItem *> m_items;

    Dialog::ActionsDialog *m_dialog{nullptr};
    Ui::ActionsDialog *m_ui{nullptr};
};

}
}
}

#endif
