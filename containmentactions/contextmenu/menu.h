/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MENU_H
#define MENU_H

// Qt
#include <QObject>

// Plasma
#include <Plasma/ContainmentActions>

class QAction;
class QMenu;

enum ViewType
{
    DockView = 0,
    PanelView
};

struct LayoutInfo {
    QString layoutName;
    bool isBackgroundFileIcon;
    QString iconName;
};

struct ViewTypeData {
    ViewType type{ViewType::DockView};
    bool isCloned{true};
    int clonesCount{0};
};

class Menu : public Plasma::ContainmentActions
{
    Q_OBJECT

public:
    Menu(QObject *parent, const QVariantList &args);
    ~Menu() override;

    QList<QAction *> contextualActions() override;
    void restore(const KConfigGroup &) override;

    QAction *action(const QString &name);
private Q_SLOTS:
    void populateLayouts();
    void populateMoveToLayouts();
    void populateViewTemplates();
    void quitApplication();
    void requestConfiguration();
    void requestWidgetExplorer();
    void updateViewData();
    void updateVisibleActions();

    void addView(QAction *action);
    void moveToLayout(QAction *action);
    void switchToLayout(QAction *action);

private:
    QStringList m_data;
    QStringList m_viewTemplates;

    QStringList m_actionsAlwaysShown;
    QStringList m_activeLayoutNames;

    ViewTypeData m_view;

    QHash<QString, QAction *> m_actions;

    QMenu *m_addViewMenu{nullptr};
    QMenu *m_switchLayoutsMenu{nullptr};
    QMenu *m_moveToLayoutMenu{nullptr};
};

#endif
