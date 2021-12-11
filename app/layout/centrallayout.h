/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2017 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CENTRALLAYOUT_H
#define CENTRALLAYOUT_H

// local
#include "genericlayout.h"
#include "../data/layoutdata.h"
#include "../wm/schemecolors.h"

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
    Q_PROPERTY(Latte::WindowSystem::SchemeColors *scheme READ scheme NOTIFY schemeChanged)

public:
    CentralLayout(QObject *parent, QString layoutFile, QString layoutName = QString());
    ~CentralLayout() override;

    bool initCorona() override;

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

    Latte::WindowSystem::SchemeColors *scheme() const;

public:
    Q_INVOKABLE bool isCurrent() override;

signals:
    void disableBordersForMaximizedWindowsChanged();
    void schemeChanged();
    void showInMenuChanged();

private slots:
    void loadConfig();
    void saveConfig();

    void onSchemeFileChanged();

private:
    void init();
    void importLocalLayout(QString file);

    void setScheme(Latte::WindowSystem::SchemeColors *_scheme);

private:
    bool m_disableBordersForMaximizedWindows{false};
    bool m_showInMenu{false};
    QStringList m_activities;

    Latte::WindowSystem::SchemeColors *m_scheme{nullptr};
};

}

#endif //CENTRALLAYOUT_H
