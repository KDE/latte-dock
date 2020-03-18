/*
 * Copyright 2017  Smith AR <audoban@openmailbox.org>
 *                 Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

// local
#include "../liblatte2/types.h"
#include "controllers/layoutscontroller.h"

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
#include <KMessageWidget>

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

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    static const int INFORMATIONINTERVAL = 3000;
    static const int INFORMATIONWITHACTIONINTERVAL = 5000;
    static const int WARNINGINTERVAL = 3500;
    static const int ERRORINTERVAL = 4000;

    SettingsDialog(QWidget *parent, Latte::Corona *corona);
    ~SettingsDialog();

    Types::LatteConfigPage currentPage();

    void toggleCurrentPage();
    void setCurrentPage(int page);

    void requestImagesDialog(int row);
    void requestColorsDialog(int row);

    void showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const int &hideInterval = 0);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void on_currentPageChanged(int page);

    void on_import_fullconfiguration();
    void on_export_fullconfiguration();

    void on_new_layout();
    void on_copy_layout();
    void on_download_layout();
    void on_pause_layout();
    void on_remove_layout();
    void on_switch_layout();
    void on_import_layout();
    void on_export_layout();
    void on_locked_layout();
    void on_shared_layout();

    void accept() override;

    void apply();
    void reset();
    void restoreDefaults();
    void showLayoutInformation();
    void showScreensInformation();
    void updateApplyButtonsState();
    void updatePerLayoutButtonsState();
    void updateWindowActivities();

private:
    void loadSettings();

    void initGlobalMenu();
    void initLayoutMenu();
    void initFileMenu();
    void initHelpMenu();

    void setTwinProperty(QAction *action, const QString &property, QVariant value);
    void twinActionWithButton(QPushButton *button, QAction *action);

    void saveAllChanges();
    void setCurrentFreeActivitiesLayout(const int &row);

    QList<int> currentSettings();

private:
    Latte::Corona *m_corona{nullptr};
    Settings::Controller::Layouts *m_layoutsController{nullptr};
    Ui::SettingsDialog *ui;

    QButtonGroup *m_inMemoryButtons;
    QButtonGroup *m_mouseSensitivityButtons;

    QAction *m_openUrlAction{nullptr};

    //! Global menu
    QMenuBar *m_globalMenuBar{nullptr};

    //! File menu actions
    QMenu *m_fileMenu{nullptr};
    QAction *m_importFullAction{nullptr};
    QAction *m_exportFullAction{nullptr};

    //! Layout menu actions
    QMenu *m_layoutMenu{nullptr};
    QAction *m_switchLayoutAction{nullptr};
    QAction *m_pauseLayoutAction{nullptr};
    QAction *m_newLayoutAction{nullptr};
    QAction *m_copyLayoutAction{nullptr};
    QAction *m_removeLayoutAction{nullptr};
    QAction *m_lockedLayoutAction{nullptr};
    QAction *m_sharedLayoutAction{nullptr};
    QAction *m_importLayoutAction{nullptr};
    QAction *m_exportLayoutAction{nullptr};
    QAction *m_downloadLayoutAction{nullptr};
    QAction *m_editLayoutAction{nullptr};

    //! Twin Actions bind QAction* behavior with QPushButton*
    QHash<QAction *, QPushButton *> m_twinActions;

    //! Help menu actions
    KHelpMenu *m_helpMenu{nullptr};

    //! workaround to assign ALLACTIVITIES during startup
    QTimer m_activitiesTimer;
    //! Timer to hide the inline message widget
    QTimer m_hideInlineMessageTimer;

    //! original data
    QList<int> o_settingsOriginalData;


};

}

#endif // SETTINGSDIALOG_H
