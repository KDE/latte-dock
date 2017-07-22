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

#ifndef LAYOUTCONFIGDIALOG_H
#define LAYOUTCONFIGDIALOG_H

#include "layoutmanager.h"
#include "layoutsettings.h"

#include <QObject>
#include <QDialog>
#include <QDebug>
#include <QStandardItemModel>

namespace Ui {
class LayoutConfigDialog;
}

namespace Latte {
class LayoutManager;
class LayoutSettings;
}

namespace Latte {

class LayoutConfigDialog : public QDialog {
    Q_OBJECT
public:
    LayoutConfigDialog(QWidget *parent, LayoutManager *corona);
    ~LayoutConfigDialog();

    QStringList activities();
    QStringList availableActivities();

private slots:
    // auto connections
    void on_copyButton_clicked();
    void on_removeButton_clicked();
    void on_switchButton_clicked();
    void on_importButton_clicked();
    void on_exportButton_clicked();

    void accept() override;
    void reject() override;
    void apply();
    void restoreDefaults();

    void currentLayoutNameChanged();
    void currentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void itemChanged(QStandardItem *item);

private:
    void loadLayouts();
    void recalculateAvailableActivities();
    void insertLayoutInfoAtRow(int row, QString path, QString color, QString name, bool menu, QStringList activities);

    bool dataAreAccepted();
    bool idExistsInModel(QString id);
    bool nameExistsInModel(QString name);
    bool saveAllChanges();

    int ascendingRowFor(QString name);

    QString uniqueTempDirectory();

    QStringList m_availableActivities;
    QStringList m_tempDirectories;
    QStringList m_initLayoutPaths;

    LayoutManager *m_manager{nullptr};

    QStandardItemModel *m_model{nullptr};
    Ui::LayoutConfigDialog *ui;

    QHash<const QString, LayoutSettings *> m_layouts;
};

}

#endif // LAYOUTCONFIGDIALOG_H
