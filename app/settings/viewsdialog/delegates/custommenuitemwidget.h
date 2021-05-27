/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUSTOMMENUITEMWIDGET_H
#define CUSTOMMENUITEMWIDGET_H

// local
#include "../../../data/screendata.h"
#include "../../../data/viewdata.h"

// Qt
#include <QAction>
#include <QWidget>
#include <QWidgetAction>
#include <QPaintEvent>
#include <QRadioButton>

namespace Latte {
namespace Settings {
namespace View {
namespace Widget {

class CustomMenuItemWidget : public QWidget {
    Q_OBJECT

public:
    CustomMenuItemWidget(QAction* action, QWidget *parent);

    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent* e) override;

    void setScreen(const Latte::Data::Screen &screen);
    void setView(const Latte::Data::View &view);

private:
    QAction *m_action{nullptr};

    Latte::Data::Screen m_screen;
    Latte::Data::View m_view;
};

}
}
}
}

#endif
