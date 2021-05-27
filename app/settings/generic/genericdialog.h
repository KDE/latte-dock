/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSGENERICDIALOG_H
#define SETTINGSGENERICDIALOG_H

// Qt
#include <QAction>
#include <QDialog>
#include <QMessageBox>
#include <QObject>
#include <QTimer>
#include <QWidget>

// KDE
#include <KMessageBox>
#include <KMessageWidget>

namespace Latte {
namespace Settings {
namespace Dialog {

class GenericDialog : public QDialog
{
    Q_OBJECT

public:
    GenericDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~GenericDialog();

    void showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const bool &isPersistent = false, QList<QAction *> actions = QList<QAction *>());
    void deleteInlineMessages();

    KMessageBox::ButtonCode saveChangesConfirmation(const QString &text);

private slots:
    KMessageWidget *initMessageWidget();
};

}
}
}

#endif
