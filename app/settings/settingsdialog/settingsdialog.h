/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2017 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

// local
#include <coretypes.h>
#include "layoutscontroller.h"
#include "tablayoutshandler.h"
#include "tabpreferenceshandler.h"
#include "../generic/genericdialog.h"

// Qt
#include <QObject>
#include <QButtonGroup>
#include <QDialog>
#include <QDebug>
#include <QMenuBar>
#include <QStandardItemModel>
#include <QTimer>

// KDE
#include <KHelpMenu>

namespace Ui {
class SettingsDialog;
}

namespace KActivities {
class Controller;
}

namespace Latte {
class Corona;
class CentralLayout;
}

namespace Latte {
namespace Settings {
namespace Dialog {

enum ConfigurationPage
{
    LayoutPage = 0,
    PreferencesPage
};

class SettingsDialog : public GenericDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent, Latte::Corona *corona);
    ~SettingsDialog();

    Latte::Corona *corona() const;
    Ui::SettingsDialog *ui() const;

    QMenuBar *appMenuBar() const;
    QMenu *fileMenu() const;
    QMenu *helpMenu() const;

    void setStoredWindowSize(const QSize &size);

    QSize downloadWindowSize() const;
    void setDownloadWindowSize(const QSize &size);

    ConfigurationPage currentPage();
    void setCurrentPage(int page);
    void toggleCurrentPage();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void reject() override;

private slots:
    void importFullConfiguration();
    void exportFullConfiguration();

    void showScreensDialog();

    void accept() override;

    void apply();
    void reset();
    void restoreDefaults();
    void showLayoutInformation();
    void showScreensInformation();
    void updateApplyButtonsState();
    void updateWindowActivities();    

    void loadConfig();
    void saveConfig();

    void onCurrentTabChanged(int index);

private:
    void initGlobalMenu();
    void initLayoutMenu();
    void initFileMenu();
    void initHelpMenu();

    void save();
    void setCurrentFreeActivitiesLayout(const int &row);

    bool saveChanges();
    QSize storedWindowSize() const;

private:
    Latte::Corona *m_corona{nullptr};
    Ui::SettingsDialog *m_ui;

    //! Handlers for UI
    Settings::Handler::TabLayouts *m_tabLayoutsHandler{nullptr};
    Settings::Handler::TabPreferences *m_tabPreferencesHandler{nullptr};

    //! properties
    QSize m_windowSize;
    QSize m_downloadWindowSize;

    //! Global menu
    QMenuBar *m_globalMenuBar{nullptr};

    //! are used for confirmation purposes, the user can choose to save or discard settings and
    //! change its current tab also
    int m_acceptedPage{-1};
    int m_nextPage{-1};

    //! File menu actions
    QMenu *m_fileMenu{nullptr};
    QAction *m_importFullAction{nullptr};
    QAction *m_exportFullAction{nullptr};

    //! Help menu actions
    KHelpMenu *m_helpMenu{nullptr};

    //! storage
    KConfigGroup m_deprecatedStorage;
    KConfigGroup m_storage;

    //! workaround to assign ALLACTIVITIES during startup
    QTimer m_activitiesTimer;
};

}
}
}

#endif // SETTINGSDIALOG_H
