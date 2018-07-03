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

#include "../liblattedock/dock.h"

#include <QObject>
#include <QButtonGroup>
#include <QDialog>
#include <QDebug>
#include <QStandardItemModel>
#include <QTimer>

namespace Ui {
class SettingsDialog;
}

namespace KActivities {
class Controller;
}

namespace Latte {
class DockCorona;
class Layout;
}

namespace Latte {

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent, DockCorona *corona);
    ~SettingsDialog();

    void setCurrentPage(Dock::LatteConfigPage page);

    QStringList activities();
    QStringList availableActivities();

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
    void on_switchButton_clicked();
    void on_importButton_clicked();
    void on_exportButton_clicked();

    void accept() override;
    void reject() override;
    void apply();
    void restoreDefaults();
    void updatePerLayoutButtonsState();

    void layoutsChanged();
    void itemChanged(QStandardItem *item);

private:
    void addLayoutForFile(QString file, QString layoutName = QString(), bool newTempDirectory = true, bool showNotification = true);
    //! When an activity is closed for some reason the window manager hides and reshows
    //! the windows. This function prevents this because we dont want to delete the window
    //! on reject in such case.
    void blockDeleteOnActivityStopped();
    void loadSettings();
    void recalculateAvailableActivities();
    void insertLayoutInfoAtRow(int row, QString path, QString color, QString textColor, QString name, bool menu, bool disabledBorders,
                               QStringList activities, bool locked = false);
    void updateApplyButtonsState();

    bool dataAreAccepted();
    bool idExistsInModel(QString id);
    bool importLayoutsFromV1ConfigFile(QString file);
    bool nameExistsInModel(QString name);
    bool saveAllChanges();

    int ascendingRowFor(QString name);

    QString uniqueTempDirectory();
    QString uniqueLayoutName(QString name);

    QList<int> currentSettings();
    QStringList currentLayoutsSettings();

private:

    QStringList m_availableActivities;
    QStringList m_tempDirectories;
    QStringList m_initLayoutPaths;

    QButtonGroup *m_inMemoryButtons;
    QButtonGroup *m_mouseSensitivityButtons;

    QTimer m_activityClosedTimer;
    bool m_blockDeleteOnReject{false};

    DockCorona *m_corona{nullptr};

    QStandardItemModel *m_model{nullptr};
    Ui::SettingsDialog *ui;

    QHash<const QString, Layout *> m_layouts;

    QList<int> o_settings;
    QStringList o_settingsLayouts;
};

}

#endif // SETTINGSDIALOG_H
