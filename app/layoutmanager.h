/*
*  Copyright 2017  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include "dockcorona.h"
#include "importer.h"
#include "layoutsettings.h"
#include "layoutconfigdialog.h"

#include <QAction>
#include <QObject>

#include <KLocalizedString>

class Importer;
class LayoutSettings;
class LayoutConfigDialog;

namespace Latte {

//! This class is responsible to manipulate all layouts.
//! add,remove,rename, update configurations etc.
class LayoutManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentLayoutName READ currentLayoutName NOTIFY currentLayoutNameChanged)

    Q_PROPERTY(QStringList layouts READ layouts NOTIFY layoutsChanged)
    Q_PROPERTY(QStringList menuLayouts READ menuLayouts NOTIFY menuLayoutsChanged)

    Q_PROPERTY(QAction *toggleLayoutAction READ toggleLayoutAction NOTIFY toggleLayoutActionChanged)
    Q_PROPERTY(QAction *addWidgetsAction READ addWidgetsAction NOTIFY addWidgetsActionChanged)

    Q_PROPERTY(LayoutSettings *currentLayout READ currentLayout NOTIFY currentLayoutChanged)

public:
    LayoutManager(QObject *parent = nullptr);
    ~LayoutManager() override;

    DockCorona *corona();
    Importer *importer();

    void load();

    QString currentLayoutName() const;

    QStringList layouts() const;
    QStringList menuLayouts() const;
    QStringList presetsPaths() const;

    QAction *addWidgetsAction();
    QAction *toggleLayoutAction();

    LayoutSettings *currentLayout();

    QStringList activities();

    void importPresets(bool includeDefault = false);

public slots:
    //! switch to specified layout
    Q_INVOKABLE bool switchToLayout(QString layoutName);

    //! creates a new layout with layoutName based on the preset
    Q_INVOKABLE QString newLayout(QString layoutName, QString preset = i18n("Default"));

    Q_INVOKABLE void showLayoutConfigDialog();

signals:
    void addWidgetsActionChanged();
    void currentLayoutChanged();
    void currentLayoutNameChanged();
    void layoutsChanged();
    void menuLayoutsChanged();
    void toggleLayoutActionChanged();

private slots:
    void showWidgetsExplorer();

private:
    void setMenuLayouts(QStringList layouts);
    QString layoutPath(QString layoutName);

    void loadLayouts();
    //! it is used to activate / deactivate the Alternative Layout
    void toggleLayout();

private:
    DockCorona *m_corona{nullptr};
    Importer *m_importer{nullptr};

    LayoutSettings *m_currentLayout{nullptr};

    QString m_lastNonAlternativeLayout{QString(i18n("My Layout"))};

    QStringList m_layouts;
    QStringList m_menuLayouts;
    QStringList m_presetsPaths;

    QAction *m_addWidgetsAction{nullptr};
    QAction *m_toggleLayoutAction{nullptr};

    QPointer<LayoutConfigDialog> m_layoutConfigDialog;

    friend class LayoutConfigDialog;
};

}

#endif // LAYOUTMANAGER_H
