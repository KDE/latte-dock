/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "genericdialog.h"

// Qt
#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QPushButton>
#include <QVBoxLayout>

// KDE
#include <KLocalizedString>
#include <KStandardGuiItem>

static const int ERRORINTERVAL = 4000;
static const int INFORMATIONINTERVAL = 3000;
static const int INFORMATIONWITHACTIONINTERVAL = 5000;
static const int POSITIVEINTERVAL = 3000;
static const int WARNINGINTERVAL = 3500;

namespace Latte {
namespace Settings {
namespace Dialog {

GenericDialog::GenericDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
}

GenericDialog::~GenericDialog()
{
}

KMessageWidget *GenericDialog::initMessageWidget()
{
    QVBoxLayout *vLayout = qobject_cast<QVBoxLayout *>(layout());

    if (!vLayout) {
        return nullptr;
    }

    auto messagewidget = new KMessageWidget(this);
    vLayout->insertWidget(vLayout->count()-1, messagewidget);
    connect(messagewidget, &KMessageWidget::hideAnimationFinished, messagewidget, &QObject::deleteLater);

    return messagewidget;
}

void GenericDialog::deleteInlineMessages()
{
    //QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(layout());

    for (int i=0; i<children().count(); ++i) {
        QObject *child = children()[i];
        KMessageWidget *messagewidget = qobject_cast<KMessageWidget *>(child);

        if(messagewidget) {
            delete messagewidget;
        }
    }
}

KMessageBox::ButtonCode GenericDialog::saveChangesConfirmation(const QString &text)
{
    QString dialogtext = text.isEmpty() ? i18n("The settings have changed.<br/>Do you want to apply the changes or discard them?") : text;

    return KMessageBox::warningYesNoCancel(this,
                                           dialogtext,
                                           i18n("Apply Settings"),
                                           KStandardGuiItem::apply(),
                                           KStandardGuiItem::discard());
}

void GenericDialog::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const bool &isPersistent, QList<QAction *> actions)
{
    if (msg.isEmpty()) {
        return;
    }

    auto messagewidget = initMessageWidget();

    if (!messagewidget) {
        return;
    }

    int hideInterval = 0;

    if (!isPersistent) {
        if (type == KMessageWidget::Error) {
            hideInterval = ERRORINTERVAL;
        } else if (type == KMessageWidget::Warning) {
            hideInterval = WARNINGINTERVAL;
        } else if (type == KMessageWidget::Information) {
            hideInterval = (actions.count() == 0 ?  INFORMATIONINTERVAL : INFORMATIONWITHACTIONINTERVAL);
        } else if (type == KMessageWidget::Positive) {
            hideInterval = POSITIVEINTERVAL;
        }
    }

    messagewidget->setCloseButtonVisible(actions.count()==0);

    if (actions.count() > 0) {
        QAction *cancelaction = new QAction(i18n("Hide"), this);
        cancelaction->setIcon(QIcon::fromTheme("dialog-cancel"));
        actions << cancelaction;
    }

    for (int i=0; i<actions.count(); ++i) {
        connect(actions[i], &QAction::triggered, messagewidget, &KMessageWidget::animatedHide);
        messagewidget->addAction(actions[i]);
    }

    messagewidget->setText(msg);

    // TODO: wrap at arbitrary character positions once QLabel can do this
    // https://bugreports.qt.io/browse/QTBUG-1276
    messagewidget->setWordWrap(true);
    messagewidget->setMessageType(type);
    messagewidget->setWordWrap(false);

    const int unwrappedWidth = messagewidget->sizeHint().width();
    messagewidget->setWordWrap(unwrappedWidth > size().width());

    // for some reason this is not smoooth so it is disabled
    //messagewidget->animatedShow();

    if (hideInterval > 0) {
        QTimer *hidetimer = new QTimer(messagewidget);
        hidetimer->setInterval(hideInterval);

        connect(hidetimer, &QTimer::timeout, this, [&, messagewidget]() {
                messagewidget->animatedHide();
        });

        hidetimer->start();
    }
}

}
}
}
