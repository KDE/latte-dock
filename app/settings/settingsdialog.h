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

    void setCurrentPage(Types::LatteConfigPage page);

    void addActivityInCurrent(const QString &activityId);
    void removeActivityFromCurrent(const QString &activityId);
    void addShareInCurrent(const QString &layoutId);
    void removeShareFromCurrent(const QString &layoutId);

    bool inMultipleLayoutsLook() const;
    bool isActive(QString layoutName) const;
    bool isShared(int row) const;
    bool isMenuCell(int column) const;

    QString nameForId(QString id) const;
    QString idForRow(int row) const;

    QStringList activities();
    QStringList availableActivities();
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
    void reject() override;
    void apply();
    void restoreDefaults();
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
    void updateActiveShares();

    bool dataAreAccepted();
    bool idExistsInModel(QString id);
    bool importLayoutsFromV1ConfigFile(QString file);
    bool mapHasRecord(const QString &record, QHash<const QString, QStringList> &map);
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

    QStringList m_availableActivities;
    QStringList m_tempDirectories;
    QStringList m_initLayoutPaths;

    QButtonGroup *m_inMemoryButtons;
    QButtonGroup *m_mouseSensitivityButtons;

    QTimer m_activityClosedTimer;
    bool m_blockDeleteOnReject{false};

    Latte::Corona *m_corona{nullptr};

    QStandardItemModel *m_model{nullptr};
    Ui::SettingsDialog *ui;

    //! SharedLayout #settingsid, Shares #settingsid
    QHash<const QString, QStringList> m_sharesMap;
    QHash<const QString, Latte::CentralLayout *> m_layouts;

    QList<int> o_settings;
    QStringList o_settingsLayouts;
};

}

#endif // SETTINGSDIALOG_H
