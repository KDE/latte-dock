/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SHAREDLAYOUT_H
#define SHAREDLAYOUT_H

// local
#include "genericlayout.h"

// Qt
#include <QObject>

namespace Latte {
class CentralLayout;
}


namespace Latte {

//! SharedLayout is a layout that exists only as long as it belongs to one or
//! more CentralLayout(s). It is a layer above a central or more layouts and can
//! be used from CentralLayouts to share Latte:View(s) . Much of its functionality
//! is provided by the CentralLayouts it belongs to. For example the activities
//! that its views should be shown is identified only from the central layouts
//! it belongs to

class SharedLayout : public Layout::GenericLayout
{
    Q_OBJECT

public:
    SharedLayout(CentralLayout *assigned, QObject *parent, QString layoutFile, QString layoutName = QString());
    ~SharedLayout() override;

    const QStringList appliedActivities() override;
    CentralLayout *currentCentralLayout() const;

    //! OVERRIDE GeneralLayout implementations
    bool isCurrent() const override;

    int viewsCount(int screen) const override;
    int viewsCount(QScreen *screen) const override;
    int viewsCount() const override;

    Layout::Type type() const override;

    void setLastConfigViewFor(Latte::View *view) override;
    Latte::View *lastConfigViewFor() override;

    //! Available edges for specific view in that screen
    QList<Plasma::Types::Location> availableEdgesForView(QScreen *scr, Latte::View *forView) const override;
    //! All free edges in that screen
    QList<Plasma::Types::Location> freeEdges(QScreen *scr) const override;
    QList<Plasma::Types::Location> freeEdges(int screen) const override;

    QList<Latte::View *> sortedLatteViews(QList<Latte::View *> views = QList<Latte::View *>()) override;

public slots:
    void addCentralLayout(CentralLayout *layout);
    void removeCentralLayout(CentralLayout *layout);

signals:
    void layoutDestroyed(SharedLayout *layout);

private slots:
    void updateLastUsedCentralLayout();

private:
    QString m_lastUsedCentralLayout;

    QList<CentralLayout *> m_centralLayouts;

};

}

#endif //SHAREDLAYOUT_H
