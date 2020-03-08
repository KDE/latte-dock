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

    void loadActivitiesInBuffer(const int &row);
    void syncActivitiesFromBuffer(const int &row);
    void addActivityInBuffer(const QString &activityId);
    void removeActivityFromBuffer(const QString &activityId);

    void addShareInCurrent(const QString &layoutId);
    void removeShareFromCurrent(const QString &layoutId);
    void updateShareAt(const int &row, const QString &fromId, const QString &toId);

    bool inMultipleLayoutsLook() const;
    bool isActive(int row) const;
    bool isActive(QString layoutName) const;
    bool isShared(int row) const;
    bool isMenuCell(int column) const;

    int currentFreeActiviesLayout() const;

    QString nameForId(QString id) const;
    QString idForRow(int row) const;

    QString freeActivities_text() const;
    QString freeActivities_icon() const;
    QString freeActivities_id() const;

    QStringList activitiesList();
    QStringList availableSharesFor(int row);

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

    void ok();
    void cancel();
    void apply();
    void restoreDefaults();
    void showLayoutInformation();
    void showScreensInformation();
    void updatePerLayoutButtonsState();

    void layoutsChanged();
    void itemChanged(QStandardItem *item);

private:
    void addLayoutForFile(QString file, QString layoutName = QString(), bool newTempDirectory = true, bool showNotification = true);
    //! When an activity is closed for some reason the window manager hides and reshows
    //! the windows. This function prevents this because we don't want to delete the window
    //! on reject in such case.
    void blockDeleteOnActivityStopped();
    void loadSettings();
    void recalculateAvailableActivities();
    void insertLayoutInfoAtRow(int row, QString path, QString color, QString textColor, QString name, bool menu, bool disabledBorders,
                               QStringList activities, bool locked = false);
    void updateApplyButtonsState();
    void updateSharedLayoutsStates();
    void updateSharedLayoutsUiElements();
    void syncActiveShares();

    void setCurrentFreeActivitiesLayout(const int &row);

    bool dataAreAccepted();
    bool idExistsInModel(QString id);
    bool importLayoutsFromV1ConfigFile(QString file);
    bool nameExistsInModel(QString name);
    bool saveAllChanges();    

    int rowForId(QString id) const;
    int rowForName(QString layoutName) const;
    int ascendingRowFor(QString name);

    QString uniqueTempDirectory();
    QString uniqueLayoutName(QString name);

    QList<int> currentSettings();
    QStringList currentLayoutsSettings();

private:
    int m_currentFreeActivitiesLayout{-1};

    QStringList m_tempDirectories;
    QStringList m_initLayoutPaths;

    QStringList m_activitiesInBuffer;

    QButtonGroup *m_inMemoryButtons;
    QButtonGroup *m_mouseSensitivityButtons;

    QTimer m_activityClosedTimer;
    bool m_blockDeleteOnReject{false};

    KHelpMenu *m_helpMenu{nullptr};

    Latte::Corona *m_corona{nullptr};

    QAction *m_editLayoutAction{nullptr};

    QStandardItemModel *m_model{nullptr};
    Ui::SettingsDialog *ui;

    //! SharedLayout #settingsid, Shares #settingsid
    QHash<const QString, QStringList> m_sharesMap;
    QHash<const QString, Latte::CentralLayout *> m_layouts;
    //! Layout #settingsid, #original_layout_name
    QHash<const QString, QString> m_originalLayoutNames;

    QList<int> o_settings;
    QStringList o_settingsLayouts;
};

}

#endif // SETTINGSDIALOG_H
