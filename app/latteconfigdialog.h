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

#ifndef LATTECONFIGDIALOG_H
#define LATTECONFIGDIALOG_H

#include "dockcorona.h"
#include "layout.h"

#include <QObject>
#include <QButtonGroup>
#include <QDialog>
#include <QDebug>
#include <QStandardItemModel>

namespace Ui {
class LatteConfigDialog;
}

namespace KActivities {
class Controller;
}

namespace Latte {
class DockCorona;
class Layout;
}

namespace Latte {

class LatteConfigDialog : public QDialog {
    Q_OBJECT
public:
    LatteConfigDialog(QWidget *parent, DockCorona *corona);
    ~LatteConfigDialog();

    QStringList activities();
    QStringList availableActivities();

private slots:
    // auto connections
    void on_newButton_clicked();
    void on_copyButton_clicked();
    void on_downloadButton_clicked();
    void on_removeButton_clicked();
    void on_switchButton_clicked();
    void on_importButton_clicked();
    void on_exportButton_clicked();

    void accept() override;
    void reject() override;
    void apply();
    void restoreDefaults();

    void layoutsChanged();
    void currentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void itemChanged(QStandardItem *item);

private:
    void addLayoutForFile(QString file, QString layoutName = QString(), bool newTempDirectory = true, bool showNotification = true);
    void loadLayouts();
    void recalculateAvailableActivities();
    void insertLayoutInfoAtRow(int row, QString path, QString color, QString name, bool menu, QStringList activities);
    void updateButtonsState();

    bool dataAreAccepted();
    bool idExistsInModel(QString id);
    bool importLayoutsFromV1ConfigFile(QString file);
    bool nameExistsInModel(QString name);
    bool saveAllChanges();

    int ascendingRowFor(QString name);

    QString uniqueTempDirectory();
    QString uniqueLayoutName(QString name);

    QStringList m_availableActivities;
    QStringList m_tempDirectories;
    QStringList m_initLayoutPaths;

    QButtonGroup *m_inMemoryButtons;

    DockCorona *m_corona{nullptr};

    QStandardItemModel *m_model{nullptr};
    Ui::LatteConfigDialog *ui;

    QHash<const QString, Layout *> m_layouts;
};

}

#endif // LATTECONFIGDIALOG_H
