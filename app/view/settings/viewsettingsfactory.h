/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWSETTINGSFACTORY_H
#define VIEWSETTINGSFACTORY_H

//Qt
#include <QObject>
#include <QPointer>

namespace Plasma {
class Containment;
}


namespace Latte {
class View;

namespace ViewPart {
class PrimaryConfigView;
class WidgetExplorerView;
}

}

namespace Latte {

class ViewSettingsFactory : public QObject
{
    Q_OBJECT

public:
    ViewSettingsFactory(QObject *parent);
    ~ViewSettingsFactory() override;

    bool hasOrphanSettings() const;
    bool hasVisibleSettings() const;

    ViewPart::PrimaryConfigView *primaryConfigView();

    Plasma::Containment *lastContainment();
    ViewPart::PrimaryConfigView *primaryConfigView(Latte::View *view);
    ViewPart::WidgetExplorerView *widgetExplorerView(Latte::View *view);

private:
    QPointer<ViewPart::PrimaryConfigView> m_primaryConfigView;
    QPointer<Plasma::Containment> m_lastContainment;

};

}

#endif
