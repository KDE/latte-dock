/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ALTERNATIVESHELPER_H
#define ALTERNATIVESHELPER_H

// Qt
#include <QQuickItem>

// Plasma
#include <Plasma/Applet>

class AlternativesHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList appletProvides READ appletProvides CONSTANT)
    Q_PROPERTY(QString currentPlugin READ currentPlugin CONSTANT)
    Q_PROPERTY(QQuickItem *applet READ applet CONSTANT)

public:
    AlternativesHelper(Plasma::Applet *applet, QObject *parent = 0);
    ~AlternativesHelper() override;

    QQuickItem *applet() const;
    QStringList appletProvides() const;
    QString currentPlugin() const;

    Q_INVOKABLE void loadAlternative(const QString &plugin);

private:
    Plasma::Applet *m_applet;
};

#endif
