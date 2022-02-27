/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONTAINMENTLAYOUTMANAGER_H
#define CONTAINMENTLAYOUTMANAGER_H

//Qt
#include <QHash>
#include <QMetaMethod>
#include <QObject>
#include <QQmlPropertyMap>
#include <QQuickItem>
#include <QTimer>

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

    Q_PROPERTY(bool hasRestoredApplets READ hasRestoredApplets NOTIFY hasRestoredAppletsChanged)

    //! this is the only way I have found to write their values properly in the configuration file in Multiple mode
    //! if they are not used from qml side in the form of plasmoid.configuration..... then
    //! appletsOrder is not stored when needed and applets additions/removals are not valid on next startup
    Q_PROPERTY(int splitterPosition READ splitterPosition NOTIFY splitterPositionChanged)
    Q_PROPERTY(int splitterPosition2 READ splitterPosition2 NOTIFY splitterPosition2Changed)
    Q_PROPERTY(QList<int> appletOrder READ appletOrder NOTIFY appletOrderChanged)
    Q_PROPERTY(QList<int> order READ order NOTIFY orderChanged) //includes also splitters
    Q_PROPERTY(QList<int> lockedZoomApplets READ lockedZoomApplets NOTIFY lockedZoomAppletsChanged)
    Q_PROPERTY(QList<int> userBlocksColorizingApplets READ userBlocksColorizingApplets NOTIFY userBlocksColorizingAppletsChanged)
    Q_PROPERTY(QList<int> appletsInScheduledDestruction READ appletsInScheduledDestruction NOTIFY appletsInScheduledDestructionChanged)

public:
    static const int JUSTIFYSPLITTERID = -10;

    LayoutManager(QObject *parent = nullptr);

    bool hasRestoredApplets() const;

    int splitterPosition() const;
    int splitterPosition2() const;
    QList<int> appletOrder() const;
    QList<int> lockedZoomApplets() const;
    QList<int> order() const;
    QList<int> userBlocksColorizingApplets() const;
    QList<int> appletsInScheduledDestruction() const;

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
    Q_INVOKABLE void setOption(const int &appletId, const QString &property, const QVariant &value);

    Q_INVOKABLE void addAppletItem(QObject *applet, int x, int y);
    Q_INVOKABLE void addAppletItem(QObject *applet, int index);
    Q_INVOKABLE void removeAppletItem(QObject *applet);

    Q_INVOKABLE void addJustifySplittersInMainLayout();
    Q_INVOKABLE void moveAppletsBasedOnJustifyAlignment();
    Q_INVOKABLE void joinLayoutsToMainLayout();
    Q_INVOKABLE void insertBefore(QQuickItem *hoveredItem, QQuickItem *item);
    Q_INVOKABLE void insertAfter(QQuickItem *hoveredItem, QQuickItem *item);
    Q_INVOKABLE void insertAtCoordinates(QQuickItem *item, const int &x, const int &y);

    Q_INVOKABLE int dndSpacerIndex();

    Q_INVOKABLE bool isMasqueradedIndex(const int &x, const int &y);
    Q_INVOKABLE int masquearadedIndex(const int &x, const int &y);
    Q_INVOKABLE QPoint indexToMasquearadedPoint(const int &index);

    void requestAppletsOrder(const QList<int> &order);
    void requestAppletsInLockedZoom(const QList<int> &applets);
    void requestAppletsDisabledColoring(const QList<int> &applets);
    void setAppletInScheduledDestruction(const int &id, const bool &enabled);


signals:
    void appletOrderChanged();
    void appletsInScheduledDestructionChanged();
    void hasRestoredAppletsChanged();
    void plasmoidChanged();
    void rootItemChanged();
    void dndSpacerChanged();
    void lockedZoomAppletsChanged();
    void userBlocksColorizingAppletsChanged();

    void mainLayoutChanged();
    void metricsChanged();
    void orderChanged();
    void splitterPositionChanged();
    void splitterPosition2Changed();
    void startLayoutChanged();
    void endLayoutChanged();

private slots:
    void onRootItemChanged();
    void destroyJustifySplitters();

    void updateOrder();
    void cleanupOptions();
    void reorderParabolicSpacers();

private:
    void restoreOptions();
    void restoreOption(const char *option);
    void saveOption(const char *option);

    void destroyAppletContainer(QObject *applet);

    void initSaveConnections();

    void insertAtLayoutTail(QQuickItem *layout, QQuickItem *item);
    void insertAtLayoutHead(QQuickItem *layout, QQuickItem *item);
    void insertAtLayoutIndex(QQuickItem *layout, QQuickItem *item, const int &index);

    void setSplitterPosition(const int &position);
    void setSplitterPosition2(const int &position);

    void setAppletOrder(const QList<int> &order);
    void setOrder(const QList<int> &order);
    void setLockedZoomApplets(const QList<int> &applets);
    void setUserBlocksColorizingApplets(const QList<int> &applets);

    void reorderSplitterInStartLayout();
    void reorderSplitterInEndLayout();

    bool isJustifySplitter(const QQuickItem *item) const;
    bool isValidApplet(const int &id);
    bool insertAtLayoutCoordinates(QQuickItem *layout, QQuickItem *item, int x, int y);

    int distanceFromTail(QQuickItem *layout, QPointF pos) const;
    int distanceFromHead(QQuickItem *layout, QPointF pos) const;

    QQuickItem *firstSplitter();
    QQuickItem *lastSplitter();
    QQuickItem *appletItem(const int &id);
    QQuickItem *appletItemInLayout(QQuickItem *layout, const int &id);

    void printAppletList(QList<QQuickItem *> list);

    QList<int> toIntList(const QString &serialized);
    QString toStr(const QList<int> &list);

private:
    //! This is needed in order to overcome plasma frameworks limitations and instead of adding dropped widgets
    //! based on coordinates, to be able to add them directly at the correct index
    static const int MASQUERADEDINDEXTOPOINTBASE = -23456;

    int m_splitterPosition{-1};
    int m_splitterPosition2{-1};
    QList<int> m_appletOrder;
    QList<int> m_lockedZoomApplets;
    QList<int> m_order;
    QList<int> m_userBlocksColorizingApplets;

    QQuickItem *m_rootItem{nullptr};
    QQuickItem *m_dndSpacer{nullptr};

    QQuickItem *m_mainLayout{nullptr};
    QQuickItem *m_startLayout{nullptr};
    QQuickItem *m_endLayout{nullptr};
    QQuickItem *m_metrics{nullptr};

    QObject *m_plasmoid{nullptr};
    KDeclarative::ConfigPropertyMap *m_configuration{nullptr};

    QHash<int, QQuickItem *> m_appletsInScheduledDestruction;

    QMetaMethod m_createAppletItemMethod;
    QMetaMethod m_createJustifySplitterMethod;
    QMetaMethod m_initAppletContainerMethod;

    bool m_hasRestoredApplets{false};
    QTimer m_hasRestoredAppletsTimer;

    //! first QString is the option in AppletItem
    //! second QString is how the option is stored in
    QHash<QString, QString> m_option;
};
}
}
#endif // CONTAINMENTLAYOUTMANAGER_H
