/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSTABLAYOUTSHANDLER_H
#define SETTINGSTABLAYOUTSHANDLER_H

//! local
#include "../generic/generichandler.h"
#include "../../data/layoutdata.h"

//! Qt
#include <QAction>
#include <QButtonGroup>
#include <QMenu>

//!
#include <KConfigGroup>

namespace Ui {
class SettingsDialog;
}

namespace Latte {
class Corona;

namespace Settings {
namespace Controller {
class Layouts;
}

namespace Dialog{
class SettingsDialog;
}

}
}

namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class TabLayouts : public Generic
{
    Q_OBJECT
public:
    TabLayouts(Dialog::SettingsDialog *parent);
    ~TabLayouts();

    bool hasChangedData() const override;
    bool inDefaultValues() const override;
    bool isCurrentTab() const;

    bool isViewsDialogVisible() const;

    Latte::Corona *corona() const;
    Dialog::SettingsDialog *dialog() const;
    Ui::SettingsDialog *ui() const;
    Controller::Layouts *layoutsController() const;

public slots:
    void onDragEnterEvent(QDragEnterEvent *event);
    void onDragLeaveEvent(QDragLeaveEvent *event);
    void onDragMoveEvent(QDragMoveEvent *event);
    void onDropEvent(QDropEvent *event);

    void showViewsDialog();

    void reset() override;
    void resetDefaults() override;
    void save() override;

signals:
    void currentPageChanged();

private slots:
    void initUi();
    void initLayoutMenu();

    void loadConfig();
    void saveConfig();

    void downloadLayout();
    void duplicateLayout();
    void switchLayout();
    void importLayout();
    void exportLayoutForBackup();
    void exportLayoutAsTemplate();
    void lockLayout();
    void removeLayout();
    void toggleActivitiesManager();
    void toggleEnabledLayout();
    void showDetailsDialog();

    void onCurrentPageChanged();
    void onLayoutFilesDropped(const QStringList &paths);
    void onRawLayoutDropped(const QString &rawLayout);
    void updatePerLayoutButtonsState();

    void newLayout(const QString &templateName);

private:
    bool isHoveringLayoutsTable(const QPoint &pos);

    void initLayoutTemplatesSubMenu();
    void initImportLayoutSubMenu();
    void initExportLayoutSubMenu();

private:
    Settings::Dialog::SettingsDialog *m_parentDialog{nullptr};
    Ui::SettingsDialog *m_ui{nullptr};
    Latte::Corona *m_corona{nullptr};

    Settings::Controller::Layouts *m_layoutsController{nullptr};

    KConfigGroup m_storage;

    bool m_isViewsDialogVisible{false};

    QButtonGroup *m_inMemoryButtons;

    //! Layout menu actions
    QMenu *m_layoutMenu{nullptr};
    QMenu *m_layoutTemplatesSubMenu{nullptr};
    QMenu *m_layoutImportSubMenu{nullptr};
    QMenu *m_layoutExportSubMenu{nullptr};

    QAction *m_switchLayoutAction{nullptr};
    QAction *m_activitiesManagerAction{nullptr};
    QAction *m_newLayoutAction{nullptr};
    QAction *m_duplicateLayoutAction{nullptr};
    QAction *m_enabledLayoutAction{nullptr};
    QAction *m_readOnlyLayoutAction{nullptr};
    QAction *m_removeLayoutAction{nullptr};
    QAction *m_importLayoutAction{nullptr};
    QAction *m_exportLayoutAction{nullptr};
    QAction *m_detailsAction{nullptr};
    QAction *m_viewsAction{nullptr};
};

}
}
}

#endif
