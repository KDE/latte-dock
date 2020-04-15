/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "genericdialog.h"

// Qt
#include <QKeyEvent>
#include <QVBoxLayout>


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
    m_hideInlineMessageTimer.setSingleShot(true);
    m_hideInlineMessageTimer.setInterval(2000);

}

GenericDialog::~GenericDialog()
{
}

void GenericDialog::initMessageWidget()
{
    if (m_messageWidget) {
        return;
    }

    QVBoxLayout *vLayout = qobject_cast<QVBoxLayout *>(layout());

    if (!vLayout) {
        return;
    }
    m_messageWidget = new KMessageWidget(this);
    vLayout->insertWidget(vLayout->count()-1, m_messageWidget);

    connect(&m_hideInlineMessageTimer, &QTimer::timeout, this, [&]() {
        m_messageWidget->animatedHide();
    });

    connect(m_messageWidget, &KMessageWidget::hideAnimationFinished, this, [&]() {
        clearCurrentMessageActions();
    });
}

void GenericDialog::keyPressEvent(QKeyEvent *event)
{
    if (event && event->key() == Qt::Key_Escape) {
        if (m_messageWidget && m_messageWidget->isVisible()) {
            m_hideInlineMessageTimer.stop();
            m_messageWidget->animatedHide();
            clearCurrentMessageActions();
            return;
        }
    }

    QDialog::keyPressEvent(event);
}

void GenericDialog::clearCurrentMessageActions()
{
    while(m_currentMessageActions.count() > 0) {
        QAction *action = m_currentMessageActions.takeAt(0);
        m_messageWidget->removeAction(action);
        action->deleteLater();
    }
}

void GenericDialog::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const bool &isPersistent, QList<QAction *> actions)
{
    if (msg.isEmpty()) {
        return;
    }

    if (!m_messageWidget) {
        initMessageWidget();
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
        }
    }

    if (!m_currentMessageActions.isEmpty()) {
        clearCurrentMessageActions();
    }

    m_currentMessageActions = actions;

    for (int i=0; i<actions.count(); ++i) {
        m_messageWidget->addAction(actions[i]);
    }

    m_hideInlineMessageTimer.stop();

    if (m_messageWidget->isVisible()) {
        m_messageWidget->animatedHide();
    }

    m_messageWidget->setText(msg);

    // TODO: wrap at arbitrary character positions once QLabel can do this
    // https://bugreports.qt.io/browse/QTBUG-1276
    m_messageWidget->setWordWrap(true);
    m_messageWidget->setMessageType(type);
    m_messageWidget->setWordWrap(false);

    const int unwrappedWidth = m_messageWidget->sizeHint().width();
    m_messageWidget->setWordWrap(unwrappedWidth > size().width());

    m_messageWidget->animatedShow();

    if (hideInterval > 0) {
        m_hideInlineMessageTimer.setInterval(hideInterval);
        m_hideInlineMessageTimer.start();
    }
}

}
}
}
