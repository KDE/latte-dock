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

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent, Latte::Corona *corona);
    ~SettingsDialog();

    void toggleCurrentPage();
    void setCurrentPage(int page);

    void requestImagesDialog(int row);
    void requestColorsDialog(int row);

private slots:
    // auto connections
    void on_newButton_clicked();
    void on_copyButton_clicked();
    void on_downloadButton_clicked();
    void on_lockedButton_clicked();
    void on_pauseButton_clicked();
    void on_removeButton_clicked();
    void on_sharedButton_clicked();
    void on_switchButton_clicked();
    void on_importButton_clicked();
    void on_exportButton_clicked();

    void accept() override;

    void apply();
    void reset();
    void restoreDefaults();
    void showLayoutInformation();
    void showScreensInformation();
    void updateApplyButtonsState();
    void updatePerLayoutButtonsState();

private:
    //! When an activity is closed for some reason the window manager hides and reshows
    //! the windows. This function prevents this because we don't want to delete the window
    //! on reject in such case.
    void blockDeleteOnActivityStopped();
    void loadSettings();

    void saveAllChanges();
    void setCurrentFreeActivitiesLayout(const int &row);

    bool dataAreAccepted();

    QList<int> currentSettings();

private:
    Latte::Corona *m_corona{nullptr};
    Settings::Controller::Layouts *m_layoutsController{nullptr};
    Ui::SettingsDialog *ui;

    QButtonGroup *m_inMemoryButtons;
    QButtonGroup *m_mouseSensitivityButtons;

    QAction *m_editLayoutAction{nullptr};

    KHelpMenu *m_helpMenu{nullptr};

    //! workaround to avoid dialog closing when kwin decides faulty to close it
    //! because of Activities changes
    QTimer m_activityClosedTimer;
    bool m_blockDeleteOnReject{false};

    //! original data
    QList<int> o_settingsOriginalData;
};

}

#endif // SETTINGSDIALOG_H
