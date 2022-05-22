/*
    SPDX-FileCopyrightText: 2022 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONTEXTMENULAYERQUICKITEM_H
#define CONTEXTMENULAYERQUICKITEM_H

// Qt
#include <QEvent>
#include <QMenu>
#include <QMetaMethod>
#include <QQuickItem>
#include <QQuickView>
#include <QPointer>
#include <QMouseEvent>
#include <QObject>

// Plasma
#include <Plasma>

namespace Plasma {
class Applet;
class Containment;
class Types;
}

namespace Latte {
class View;
}

namespace Latte {

class ContextMenuLayerQuickItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool menuIsShown READ menuIsShown NOTIFY menuChanged)
    Q_PROPERTY(QObject *view READ view WRITE setView NOTIFY viewChanged)

public:
    ContextMenuLayerQuickItem(QQuickItem *parent = nullptr);
    ~ContextMenuLayerQuickItem() override;

    QObject *view() const;
    void setView(QObject *view);

    bool menuIsShown() const;

signals:
    void menuChanged();
    void viewChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onMenuAboutToHide();

private:
    void addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu *desktopMenu, QEvent *event);
    void updateAppletContainsMethod();

    QPoint popUpRelevantToParent(const QRect &parentItem, const QRect popUpRect);
    QPoint popUpRelevantToGlobalPoint(const QRect &parentItem, const QRect popUpRect);

    QPoint popUpTopLeft(Plasma::Applet *applet, const QRect popUpRect);

    Plasma::Containment *containmentById(uint id);


private:
    Plasma::Types::ItemStatus m_lastContainmentStatus;

    QPointer<QMenu> m_contextMenu;
    QMetaMethod m_appletContainsMethod;
    QQuickItem *m_appletContainsMethodItem{nullptr};

    Latte::View *m_latteView{nullptr};

    friend class Latte::View;
};

}

#endif // DOCKMENUMANAGER_H
