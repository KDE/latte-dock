/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SHORTCUTSTRACKER_H
#define SHORTCUTSTRACKER_H

// Qt
#include <QObject>

// KDE
#include <KSharedConfig>

namespace Latte {
namespace ShortcutsPart {

class ShortcutsTracker: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool basedOnPositionEnabled READ basedOnPositionEnabled NOTIFY badgesForActivateChanged)
    Q_PROPERTY(QStringList badgesForActivate READ badgesForActivate NOTIFY badgesForActivateChanged)

public:
    ShortcutsTracker(QObject *parent);
    ~ShortcutsTracker() override;

    void clearAllAppletShortcuts();

    bool basedOnPositionEnabled() const;

    QStringList badgesForActivate() const;

    QList<uint> appletsWithPlasmaShortcuts();

public slots:
    Q_INVOKABLE QString appletShortcutBadge(int appletId);

signals:
    void badgesForActivateChanged();

private slots:
    void shortcutsFileChanged(const QString &file);

private:
    void initGlobalShortcutsWatcher();
    //! access user set global shortcuts for activate entries
    void parseGlobalShortcuts();

    QString shortcutToBadge(QStringList shortcutRecords);

private:
    bool m_basedOnPositionEnabled{false};

    QStringList m_badgesForActivate;

    //! shortcuts assigned to applets through plasma infrastructure
    //! <applet id, shortcut>
    QHash<uint, QString> m_appletShortcuts;

    KSharedConfig::Ptr m_shortcutsConfigPtr;
};

}
}

#endif
