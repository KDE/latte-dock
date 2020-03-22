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
#include "handlers/tablayoutshandler.h"
#include "handlers/tabpreferenceshandler.h"

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

    Latte::Corona *corona() const;
    Ui::SettingsDialog *ui() const;
    QMenuBar *appMenuBar() const;

    Types::LatteConfigPage currentPage();
    void setCurrentPage(int page);
    void toggleCurrentPage();

    void requestImagesDialog(int row);
    void requestColorsDialog(int row);

    void showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const int &hideInterval = 0, QList<QAction *> actions = QList<QAction *>());

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void on_import_fullconfiguration();
    void on_export_fullconfiguration();

    void accept() override;

    void apply();
    void reset();
    void restoreDefaults();
    void showLayoutInformation();
    void showScreensInformation();
    void updateApplyButtonsState();
    void updateWindowActivities();

    void clearCurrentMessageActions();

private:
    void initGlobalMenu();
    void initLayoutMenu();
    void initFileMenu();
    void initHelpMenu();

    void save();
    void setCurrentFreeActivitiesLayout(const int &row);

private:
    Latte::Corona *m_corona{nullptr};
    Ui::SettingsDialog *m_ui;

    //! Handlers for UI
    Settings::Handler::TabLayouts *m_tabLayoutsHandler{nullptr};
    Settings::Handler::TabPreferences *m_preferencesHandler{nullptr};

    //! Global menu
    QMenuBar *m_globalMenuBar{nullptr};

    //! File menu actions
    QMenu *m_fileMenu{nullptr};
    QAction *m_importFullAction{nullptr};
    QAction *m_exportFullAction{nullptr};

    //! Help menu actions
    KHelpMenu *m_helpMenu{nullptr};

    //! Current shown KMessageActions
    QList<QAction *> m_currentMessageActions;

    //! workaround to assign ALLACTIVITIES during startup
    QTimer m_activitiesTimer;
    //! Timer to hide the inline message widget
    QTimer m_hideInlineMessageTimer;

};

}

#endif // SETTINGSDIALOG_H
