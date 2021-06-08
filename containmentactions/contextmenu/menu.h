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

struct LayoutInfo {
    QString layoutName;
    bool isBackgroundFileIcon;
    QString iconName;
};

class Menu : public Plasma::ContainmentActions
{
    Q_OBJECT

public:
    Menu(QObject *parent, const QVariantList &args);
    ~Menu() override;

    QList<QAction *> contextualActions() override;

    QAction *action(const QString &name);
private Q_SLOTS:
    void makeActions();
    void onUserConfiguringChanged(const bool &configuring);
    void populateLayouts();
    void populateMoveToLayouts();
    void populateViewTemplates();
    void quitApplication();
    void requestConfiguration();
    void requestWidgetExplorer();
    void updateVisibleActions();

    void addView(QAction *action);
    void moveToLayout(QAction *action);
    void switchToLayout(QAction *action);

private:
    QStringList m_data;
    QStringList m_viewTemplates;

    QStringList m_actionsAlwaysShown;

    QHash<QString, QAction *> m_actions;

    QAction *m_separator{nullptr};

    QAction *m_addWidgetsAction{nullptr};
    QAction *m_configureAction{nullptr};
    QAction *m_duplicateAction{nullptr};
    QAction *m_exportViewAction{nullptr};
    QAction *m_preferenceAction{nullptr};
    QAction *m_printAction{nullptr};
    QAction *m_removeAction{nullptr};
    QAction *m_quitApplication{nullptr};

    QAction *m_addViewAction{nullptr};
    QMenu *m_addViewMenu{nullptr};

    QAction *m_layoutsAction{nullptr};
    QMenu *m_switchLayoutsMenu{nullptr};

    QAction *m_moveAction{nullptr};
    QMenu *m_moveToLayoutMenu{nullptr};
};

#endif
