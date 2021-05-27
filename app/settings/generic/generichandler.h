/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSGENERICHANDLER_H
#define SETTINGSGENERICHANDLER_H

//! Qt
#include <QAction>
#include <QObject>
#include <QPushButton>

// KDE
#include <KMessageWidget>

namespace Latte {
namespace Settings {
namespace Dialog {
class GenericDialog;
}
}
}


namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class Generic : public QObject
{
    Q_OBJECT
public:
    static constexpr const char* TWINENABLED = "Enabled";
    static constexpr const char* TWINVISIBLE = "Visible";
    static constexpr const char* TWINCHECKED = "Checked";

    Generic(Dialog::GenericDialog *parent);
    Generic(Dialog::GenericDialog *parentDialog, QObject *parent);

    virtual bool hasChangedData() const = 0;
    virtual bool inDefaultValues() const = 0;

    void showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const bool &isPersistent = false, QList<QAction *> actions = QList<QAction *>());

public slots:
    virtual void reset() = 0;
    virtual void resetDefaults() = 0;
    virtual void save() = 0;

signals:
    void dataChanged();

protected:
    void setTwinProperty(QAction *action, const QString &property, QVariant value);
    void connectActionWithButton(QPushButton *button, QAction *action);

private:
    //! Twin Actions bind QAction* behavior with QPushButton*
    //! for simplicity reasons
    QHash<QAction *, QPushButton *> m_twinActions;

    Dialog::GenericDialog *m_dialog;


};

}
}
}

#endif
