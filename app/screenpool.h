/*
 *  Copyright 2016 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
    ScreenPool(KSharedConfig::Ptr config, QObject *parent = nullptr);
    void load();
    ~ScreenPool() override;

    bool hasScreenId(int screenId) const;
    bool isScreenActive(int screenId) const;
    int primaryScreenId() const;

    void insertScreenMapping(const QString &connector);
    void reload(QString path);

    int id(const QString &connector) const;

    QString connector(int id) const;

    QString reportHtml(const QList<int> &assignedScreens) const;

    QScreen *screenForId(int id);

signals:
    void primaryPoolChanged();

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;

    int firstAvailableId() const;

private slots:
    void updateScreenGeometry(const int &screenId, const QRect &screenGeometry);

private:
    void save();
    Latte::Data::ScreensTable m_screensTable;

    KConfigGroup m_configGroup;
    //! used to workaround a bug under X11 when primary screen changes and no screenChanged signal is emitted
    QString m_lastPrimaryConnector;

    //order is important
  //  QMap<int, QString> m_connectorForId;
   // QHash<QString, int> m_idForConnector;

    QTimer m_configSaveTimer;
};

}

#endif // SCREENPOOL_H
