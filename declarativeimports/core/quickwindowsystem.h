/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QUICKWINDOWSYSTEM_H
#define QUICKWINDOWSYSTEM_H

// Qt
#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>

namespace Latte {

/**
 * @brief The QuickWindowSystem class,
 * is a tiny class that provide basic information of WindowSystem
 */
class QuickWindowSystem final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool compositingActive READ compositingActive NOTIFY compositingChanged FINAL)
    Q_PROPERTY(bool isPlatformWayland READ isPlatformWayland NOTIFY isPlatformWaylandChanged FINAL)
    Q_PROPERTY(bool isPlatformX11 READ isPlatformX11 NOTIFY isPlatformX11Changed FINAL)

public:
    explicit QuickWindowSystem(QObject *parent = nullptr);
    virtual ~QuickWindowSystem();

    bool compositingActive() const;
    bool isPlatformWayland() const;
    bool isPlatformX11() const;

signals:
    void compositingChanged();
    void isPlatformWaylandChanged();
    void isPlatformX11Changed();

private:
    bool m_compositing{true};
};

static QObject *windowsystem_qobject_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

// NOTE: QML engine is the owner of this resource
    return new QuickWindowSystem;
}

}

#endif // QUICKWINDOWSYSTEM_H
