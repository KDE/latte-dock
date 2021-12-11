/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SCREENPOOL_H
#define SCREENPOOL_H

// local
#include "data/screendata.h"

// Qt
#include <QObject>
#include <QHash>
#include <QScreen>
#include <QString>
#include <QTimer>
#include <QAbstractNativeEventFilter>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

namespace Latte {

class ScreenPool : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    static const int FIRSTSCREENID = 10;
    static const int NOSCREENID = -1;

    ScreenPool(KSharedConfig::Ptr config, QObject *parent = nullptr);
    void load();
    ~ScreenPool() override;

    bool hasScreenId(int screenId) const;
    bool isScreenActive(int screenId) const;
    int primaryScreenId() const;
    QList<int> secondaryScreenIds() const;

    void insertScreenMapping(const QString &connector);
    void reload(QString path);
    void removeScreens(const Latte::Data::ScreensTable &obsoleteScreens);

    int id(const QString &connector) const;

    QString connector(int id) const;

    QScreen *screenForId(int id);

    Latte::Data::ScreensTable screensTable();

signals:
    void primaryPoolChanged();
    void screenGeometryChanged();

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;

    int firstAvailableId() const;

private slots:
    void updateScreenGeometry(const QScreen *screen);
    void onScreenAdded(const QScreen *screen);
    void onScreenRemoved(const QScreen *screen);

private:
    void save();
    void updateScreenGeometry(const int &screenId, const QRect &screenGeometry);

private:
    Latte::Data::ScreensTable m_screensTable;

    KConfigGroup m_configGroup;
    //! used to workaround a bug under X11 when primary screen changes and no screenChanged signal is emitted
    QString m_lastPrimaryConnector;

    QTimer m_configSaveTimer;
};

}

#endif // SCREENPOOL_H
