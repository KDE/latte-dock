/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTMENUITEMWIDGET_H
#define LAYOUTMENUITEMWIDGET_H

// Qt
#include <QAction>
#include <QWidget>
#include <QWidgetAction>
#include <QPaintEvent>
#include <QStyleOptionMenuItem>


class LayoutMenuItemWidget : public QWidget {
    Q_OBJECT

public:
    LayoutMenuItemWidget(QAction* action, QWidget *parent);

    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent* e) override;

    void setIcon(const bool &isBackgroundFile, const QString &iconName);

private:
    QAction *m_action{nullptr};
    bool m_isBackgroundFile;
    QString m_iconName;
};

#endif
