/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BACKGROUNDTRACKER_H
#define BACKGROUNDTRACKER_H

// Qt
#include <QObject>

// Plasma
#include <Plasma>

namespace Latte{

class BackgroundTracker: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)

    Q_PROPERTY(int location READ location WRITE setLocation NOTIFY locationChanged)

    Q_PROPERTY(float currentBrightness READ currentBrightness NOTIFY currentBrightnessChanged)

    Q_PROPERTY(QString activity READ activity WRITE setActivity NOTIFY activityChanged)
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName NOTIFY screenNameChanged)

public:
    BackgroundTracker(QObject *parent = nullptr);
    virtual ~BackgroundTracker();

    bool isBusy() const;

    int location() const;
    void setLocation(int location);

    float currentBrightness() const;

    QString activity() const;
    void setActivity(QString id);

    QString screenName() const;
    void setScreenName(QString name);

signals:
    void activityChanged();
    void currentBrightnessChanged();
    void isBusyChanged();
    void locationChanged();
    void screenNameChanged();

private slots:
    void backgroundChanged(const QString &activity, const QString &screenName);
    void update();

private:
    // local
    bool m_busy{false};
    float m_brightness{-1000};

    // Qt
    QString m_activity;
    QString m_screenName;

    // Plasma
    Plasma::Types::Location m_location{Plasma::Types::BottomEdge};

};

}

#endif
