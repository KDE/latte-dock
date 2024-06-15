/*
    SPDX-FileCopyrightText: 2013 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CONFIGPROPERTYMAP_H
#define CONFIGPROPERTYMAP_H

#include <QQmlPropertyMap>

class KCoreConfigSkeleton;

namespace Latte {
namespace Legacy {
class ConfigPropertyMapPrivate;

/**
 * @class KDeclarative::ConfigPropertyMap configpropertymap.h KDeclarative/ConfigPropertyMap
 *
 * An object that (optionally) automatically saves changes in a
 * property map to a configuration object (e.g. a KConfig file).
 * @deprecated Since 5.91, use KConfigPropertyMap from KConfig KConfigQml module instead.
 * The replacement class does not provide any autosave behavior, instead it requires
 * an explicit writeConfig call.
 */
class ConfigPropertyMap : public QQmlPropertyMap
{
    Q_OBJECT

public:
    ConfigPropertyMap(KCoreConfigSkeleton *config, QObject *parent = nullptr);
    ~ConfigPropertyMap() override;

    /**
     * Indicates if the map calls save() on the config object on each write or not
     *
     * @return true if the map automatically saves (the default), false otherwise
     * @since 5.65
     */
    bool isAutosave() const;

    /**
     * Allows to set the map into autosave mode or not
     *
     * @param autosave the new value for autosaving
     * @since 5.65
     */
    void setAutosave(bool autosave);

    /**
     * Whether notifications on config changes are enabled. Disabled by default.
     * @see KConfigBase::Notify
     * @return true if writes send (dbus) notifications
     * @since 5.73
     */
    bool isNotify() const;

    /**
     * Enable or disable notifications on config changes.
     * @see KConfigBase::Notify
     * @param notify whether to send notifications
     * @since 5.73
     */
    void setNotify(bool notify);

    /**
     * @brief Whether the value at the given key is immutable
     *
     * @return true if the value is immutable, false if it isn't or it doesn't exist
     */
    Q_INVOKABLE bool isImmutable(const QString &key) const;

protected:
    QVariant updateValue(const QString &key, const QVariant &input) override;

private:
    ConfigPropertyMapPrivate *const d;
};

}
}

#endif
