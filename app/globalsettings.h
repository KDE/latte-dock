/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
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

#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include "dockcorona.h"
#include "../liblattedock/dock.h"

#include <QFileDialog>
#include <QPointer>

#include <KConfigGroup>
#include <KSharedConfig>

class DockCorona;

namespace Latte {

class GlobalSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY autostartChanged)
    Q_PROPERTY(bool exposeAltSession READ exposeAltSession WRITE setExposeAltSession NOTIFY exposeAltSessionChanged)

    Q_PROPERTY(bool syncLaunchers READ syncLaunchers WRITE setSyncLaunchers NOTIFY syncLaunchersChanged)
    Q_PROPERTY(QStringList globalLaunchers READ globalLaunchers WRITE setGlobalLaunchers NOTIFY globalLaunchersChanged)

    Q_PROPERTY(Latte::Dock::SessionType currentSession READ currentSession WRITE setCurrentSession NOTIFY currentSessionChanged)

    Q_PROPERTY(QAction *altSessionAction READ altSessionAction NOTIFY altSessionActionChanged)
    Q_PROPERTY(QAction *addWidgetsAction READ addWidgetsAction NOTIFY addWidgetsActionChanged)

public:
    GlobalSettings(QObject *parent = nullptr);
    ~GlobalSettings() override;

    void load();

    bool autostart() const;
    void setAutostart(bool state);

    bool exposeAltSession() const;
    void setExposeAltSession(bool state);
    QAction *altSessionAction() const;

    bool syncLaunchers() const;
    void setSyncLaunchers(bool sync);

    QStringList globalLaunchers() const;
    void setGlobalLaunchers(QStringList launchers);

    QAction *addWidgetsAction() const;

    Latte::Dock::SessionType currentSession() const;
    void setCurrentSession(Latte::Dock::SessionType session);

    static bool importHelper(const QString &fileName);
    Q_INVOKABLE void importConfiguration();
    Q_INVOKABLE void exportConfiguration();
    Q_INVOKABLE void importLayout(const QString &name, const QString &file);
    Q_INVOKABLE QVariantList layouts();

signals:
    void addWidgetsActionChanged();
    void altSessionActionChanged();
    void autostartChanged();
    void clearLayoutSelection();
    void currentSessionChanged();
    void exposeAltSessionChanged();
    void globalLaunchersChanged();
    void syncLaunchersChanged();

private slots:
    void currentSessionChangedSlot(Dock::SessionType type);
    void enableAltSession(bool enabled);
    void importLayoutInternal(const QString &file);
    void showWidgetsExplorer();

private:
    void save();
    void init();
    void initExtConfiguration();
    void loadExtConfiguration();
    void saveExtConfiguration();

    bool m_exposeAltSession{false};
    bool m_syncLaunchers{false};

    QAction *m_addWidgetsAction{nullptr};
    QAction *m_altSessionAction{nullptr};
    DockCorona *m_corona{nullptr};
    QPointer<QFileDialog> m_fileDialog;
    QVariantList m_defaultLayouts;
    QVariantList m_userLayouts;
    QStringList m_userLayoutsFiles;
    QStringList m_globalLaunchers;

    KConfigGroup m_configGroup;
    KConfigGroup m_externalGroup;
};

}

#endif // GLOBALSETTINGS_H
