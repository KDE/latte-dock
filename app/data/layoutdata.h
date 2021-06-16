/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSDATALAYOUT_H
#define SETTINGSDATALAYOUT_H

// local
#include "genericdata.h"
#include "viewstable.h"
#include "../layout/abstractlayout.h"

//Qt
#include <QMetaType>
#include <QString>
#include <QStringList>

namespace Latte {
namespace Data {

class Layout : public Generic
{
public:
    static constexpr const char* ALLACTIVITIESID = "{0}";
    static constexpr const char* FREEACTIVITIESID = "{free-activities}";
    static constexpr const char* CURRENTACTIVITYID = "{current-activity}";
    static constexpr const char* DEFAULTSCHEMEFILE = "kdeglobals";

    Layout();
    Layout(Layout &&o);
    Layout(const Layout &o);

    //! Layout data
    QString icon;
    QString color;
    QString background;
    QString textColor;
    QString lastUsedActivity;
    QString schemeFile{DEFAULTSCHEMEFILE};
    bool isActive{false};
    bool isConsideredActive{false}; //used from settings window to indicate activeness based on selected layouts mode
    bool isLocked{false};
    bool isShownInMenu{false};
    bool isTemplate{false};
    bool hasDisabledBorders{false};
    int popUpMargin{-1};
    QStringList activities;
    int errors{0};
    int warnings{0};

    Latte::Layout::BackgroundStyle backgroundStyle{Latte::Layout::ColorBackgroundStyle};

    ViewsTable views;

    //! Functionality
    bool isOnAllActivities() const;
    bool isForFreeActivities() const;
    bool isTemporary() const;
    bool isNull() const;
    bool isEmpty() const;
    bool isSystemTemplate() const;

    bool hasErrors() const;
    bool hasWarnings() const;

    //! Operators
    Layout &operator=(const Layout &rhs);
    Layout &operator=(Layout &&rhs);
    bool operator==(const Layout &rhs) const;
    bool operator!=(const Layout &rhs) const;
};

}
}

Q_DECLARE_METATYPE(Latte::Data::Layout)

#endif
