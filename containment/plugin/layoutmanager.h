/*
*  Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
*
*
*  This file is part of Latte-Dock and is a Fork of PlasmaCore::IconItem
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

#ifndef CONTAINMENTLAYOUTMANAGER_H
#define CONTAINMENTLAYOUTMANAGER_H

//Qt
#include <QHash>
#include <QMetaMethod>
#include <QObject>
#include <QQmlPropertyMap>
#include <QQuickItem>

namespace KDeclarative {
class ConfigPropertyMap;
}

namespace Latte{
namespace Containment{

class LayoutManager : public QObject
{
  Q_OBJECT
    Q_PROPERTY(QObject *plasmoidObj READ plasmoid() WRITE setPlasmoid NOTIFY plasmoidChanged)

    Q_PROPERTY(QQuickItem *rootItem READ rootItem WRITE setRootItem NOTIFY rootItemChanged)
    Q_PROPERTY(QQuickItem *mainLayout READ mainLayout WRITE setMainLayout NOTIFY mainLayoutChanged)
    Q_PROPERTY(QQuickItem *startLayout READ startLayout WRITE setStartLayout NOTIFY startLayoutChanged)
    Q_PROPERTY(QQuickItem *endLayout READ endLayout WRITE setEndLayout NOTIFY endLayoutChanged)

    Q_PROPERTY(QQuickItem *dndSpacerItem READ dndSpacer WRITE setDndSpacer NOTIFY dndSpacerChanged)
    Q_PROPERTY(QQuickItem *metrics READ metrics WRITE setMetrics NOTIFY metricsChanged)

    //! this is the only way I have found to write their values properly in the configuration file in Multiple mode
    //! if they are not used from qml side in the form of plasmoid.configuration..... then
    //! appletsOrder is not stored when needed and applets additions/removals are not valid on next startup
    Q_PROPERTY(int splitterPosition READ splitterPosition NOTIFY splitterPositionChanged)
    Q_PROPERTY(int splitterPosition2 READ splitterPosition2 NOTIFY splitterPosition2Changed)
    Q_PROPERTY(QString appletOrder READ appletOrder NOTIFY appletOrderChanged)
    Q_PROPERTY(QString lockedZoomApplets READ lockedZoomApplets NOTIFY lockedZoomAppletsChanged)
    Q_PROPERTY(QString userBlocksColorizingApplets READ userBlocksColorizingApplets NOTIFY userBlocksColorizingAppletsChanged)

public:
    LayoutManager(QObject *parent = nullptr);

    int splitterPosition() const;
    int splitterPosition2() const;
    QString appletOrder() const;
    QString lockedZoomApplets() const;
    QString userBlocksColorizingApplets() const;

    QObject *plasmoid() const;
    void setPlasmoid(QObject *plasmoid);

    QQuickItem *rootItem() const;
    void setRootItem(QQuickItem *root);

    QQuickItem *mainLayout() const;
    void setMainLayout(QQuickItem *main);

    QQuickItem *startLayout() const;
    void setStartLayout(QQuickItem *start);

    QQuickItem *endLayout() const;
    void setEndLayout(QQuickItem *end);

    QQuickItem *dndSpacer() const;
    void setDndSpacer(QQuickItem *dnd);

    QQuickItem *metrics() const;
    void setMetrics(QQuickItem *metrics);

public slots:
    Q_INVOKABLE void restore();
    Q_INVOKABLE void save();
    Q_INVOKABLE void saveOptions();

    Q_INVOKABLE void addAppletItem(QObject *applet, int x, int y);
    Q_INVOKABLE void removeAppletItem(QObject *applet);

    Q_INVOKABLE void addJustifySplittersInMainLayout();
    Q_INVOKABLE void moveAppletsBasedOnJustifyAlignment();
    Q_INVOKABLE void joinLayoutsToMainLayout();
    Q_INVOKABLE void insertBefore(QQuickItem *hoveredItem, QQuickItem *item);
    Q_INVOKABLE void insertAfter(QQuickItem *hoveredItem, QQuickItem *item);
    Q_INVOKABLE void insertAtCoordinates(QQuickItem *item, const int &x, const int &y);

signals:
    void appletOrderChanged();
    void plasmoidChanged();
    void rootItemChanged();
    void dndSpacerChanged();
    void lockedZoomAppletsChanged();
    void userBlocksColorizingAppletsChanged();

    void mainLayoutChanged();
    void metricsChanged();
    void splitterPositionChanged();
    void splitterPosition2Changed();
    void startLayoutChanged();
    void endLayoutChanged();

private slots:
    void onRootItemChanged();
    void destroyJustifySplitters();

private:
    void restoreOptions();
    void restoreOption(const char *option);
    void saveOption(const char *option);

    void setSplitterPosition(const int &position);
    void setSplitterPosition2(const int &position);

    void setAppletOrder(const QString &order);
    void setLockedZoomApplets(const QString &applets);
    void setUserBlocksColorizingApplets(const QString &applets);

    void reorderSplitterInStartLayout();
    void reorderSplitterInEndLayout();

    bool isValidApplet(const int &id);
    bool insertAtLayoutCoordinates(QQuickItem *layout, QQuickItem *item, int x, int y);

private:
    int m_splitterPosition{-1};
    int m_splitterPosition2{-1};
    QString m_appletOrder;
    QString m_lockedZoomApplets;
    QString m_userBlocksColorizingApplets;

    QQuickItem *m_rootItem{nullptr};
    QQuickItem *m_dndSpacer{nullptr};

    QQuickItem *m_mainLayout{nullptr};
    QQuickItem *m_startLayout{nullptr};
    QQuickItem *m_endLayout{nullptr};
    QQuickItem *m_metrics{nullptr};

    QObject *m_plasmoid{nullptr};
    KDeclarative::ConfigPropertyMap *m_configuration{nullptr};

    QMetaMethod m_createAppletItemMethod;
    QMetaMethod m_createJustifySplitterMethod;

    //! first QString is the option in AppletItem
    //! second QString is how the option is stored in
    QHash<QString, QString> m_option;
};
}
}
#endif // CONTAINMENTLAYOUTMANAGER_H
