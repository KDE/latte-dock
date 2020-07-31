/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef INDICATORUIMANAGER_H
#define INDICATORUIMANAGER_H

//Qt
#include <QList>
#include <QObject>
#include <QQuickItem>
#include <QPointer>

namespace KDeclarative
{
class QmlObjectSharedEngine;
}

namespace Latte {
class View;
}

namespace Latte {
namespace ViewPart {
class PrimaryConfigView;
}
}

namespace Latte {
namespace ViewPart {
namespace Config {

struct IndicatorUiData
{
    QString type;
    QString pluginPath;
    QPointer<Latte::View> view;
    QPointer<KDeclarative::QmlObjectSharedEngine> ui;
};

class IndicatorUiManager : public QObject
{
    Q_OBJECT

public:
    IndicatorUiManager(ViewPart::PrimaryConfigView *parent);
    ~IndicatorUiManager() override;

public slots:
    Q_INVOKABLE void addIndicator();
    Q_INVOKABLE void downloadIndicator();
    Q_INVOKABLE void removeIndicator(QString pluginId);
    Q_INVOKABLE void setParentItem(QQuickItem *parentItem);
    Q_INVOKABLE void ui(const QString &type, Latte::View *view);

private:
    bool contains(const QString &type);
    int index(const QString &type);

    void hideAllUi();

private:
    QQuickItem *m_parentItem{nullptr};
    PrimaryConfigView *m_primary{nullptr};

    QList<IndicatorUiData> m_uidata;

};

}
}
}

#endif

