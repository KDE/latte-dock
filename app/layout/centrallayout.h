/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CENTRALLAYOUT_H
#define CENTRALLAYOUT_H

// local
#include "genericlayout.h"
#include "../data/layoutdata.h"

// Qt
#include <QObject>

namespace Latte {
class Corona;
}

namespace Latte {

//! CentralLayout is a layout that is assigned to ALL Activities, FREE Activities or SPRECIFIC Activities.
//! It is a real running layout instance.
//!
//! It holds all the important settings in order to provide specific
//! behavior for the Activities is assigned at.
//! for example: activities for which its views should be shown,
//! if the maximized windows will be borderless,
//! if the layout will be shown at user layout contextmenu.
//!

class CentralLayout : public Layout::GenericLayout
{
    Q_OBJECT

public:
    CentralLayout(QObject *parent, QString layoutFile, QString layoutName = QString());
    ~CentralLayout() override;

    void initToCorona(Latte::Corona *corona);

    bool disableBordersForMaximizedWindows() const;
    void setDisableBordersForMaximizedWindows(bool disable);

    bool showInMenu() const;
    void setShowInMenu(bool show);

    bool isForFreeActivities() const;
    bool isOnAllActivities() const;

    QStringList activities() const;
    void setActivities(QStringList activities);

    const QStringList appliedActivities() override;
    Types::ViewType latteViewType(uint containmentId) const override;

    Layout::Type type() const override;
    Data::Layout data() const;

public:
    Q_INVOKABLE bool isCurrent() override;

signals:
    void disableBordersForMaximizedWindowsChanged();
    void showInMenuChanged();

private slots:
    void loadConfig();
    void saveConfig();

private:
    void init();
    void importLocalLayout(QString file);

private:
    bool m_disableBordersForMaximizedWindows{false};
    bool m_showInMenu{false};
    QStringList m_activities;
};

}

#endif //CENTRALLAYOUT_H
