/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PRIMARYOUTPUTWATCHER_H
#define PRIMARYOUTPUTWATCHER_H

#include <QAbstractNativeEventFilter>
#include <QObject>

class QScreen;

class PrimaryOutputWatcher : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    PrimaryOutputWatcher(QObject *parent);
    QScreen *primaryScreen() const;
    QScreen *screenForName(const QString &outputName) const;

Q_SIGNALS:
    void primaryOutputNameChanged(const QString &oldOutputName, const QString &newOutputName);

protected:
    friend class WaylandOutputDevice;
    void setPrimaryOutputName(const QString &outputName);

private:
    void setupWaylandIntegration();
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

    // All
    QString m_primaryOutputName;

    QString m_primaryOutputWayland;

    // Xrandr
    int m_xrandrExtensionOffset;
};

#endif // PRIMARYOUTPUTWATCHER_H
