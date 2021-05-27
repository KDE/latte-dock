/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

// Qt
#include <QEvent>
#include <QMenu>
#include <QMetaMethod>
#include <QQuickItem>
#include <QQuickView>
#include <QPointer>
#include <QMouseEvent>
#include <QObject>

namespace Plasma {
class Applet;
class Containment;
}

namespace Latte {
class View;
}

namespace Latte {
namespace ViewPart {

class ContextMenu : public QObject
{
    Q_OBJECT

public:
    ContextMenu(Latte::View *view);
    ~ContextMenu() override;

    QMenu *menu();

    bool mousePressEvent(QMouseEvent *event);
    bool mousePressEventForContainmentMenu(QQuickView *view, QMouseEvent *event);

signals:
    void menuChanged();

private slots:
    void menuAboutToHide();

private:
    void addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu *desktopMenu, QEvent *event);
    void updateAppletContainsMethod();

    QPoint popUpRelevantToParent(const QRect &parentItem, const QRect popUpRect);
    QPoint popUpRelevantToGlobalPoint(const QRect &parentItem, const QRect popUpRect);

    QPoint popUpTopLeft(Plasma::Applet *applet, const QRect popUpRect);

    Plasma::Containment *containmentById(uint id);


private:
    QPointer<QMenu> m_contextMenu;
    QMetaMethod m_appletContainsMethod;
    QQuickItem *m_appletContainsMethodItem{nullptr};

    Latte::View *m_latteView;

    friend class Latte::View;
};

}
}

#endif // DOCKMENUMANAGER_H
