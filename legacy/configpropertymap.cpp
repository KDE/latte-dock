/*
    SPDX-FileCopyrightText: 2013 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configpropertymap.h"

#include <KCoreConfigSkeleton>
#include <QJSValue>
#include <QPointer>

namespace Latte {
namespace Legacy {

class ConfigPropertyMapPrivate
{
public:
    ConfigPropertyMapPrivate(ConfigPropertyMap *map)
        : q(map)
    {
    }

    enum LoadConfigOption {
        DontEmitValueChanged,
        EmitValueChanged,
    };

    void loadConfig(LoadConfigOption option);
    void writeConfig();
    void writeConfigValue(const QString &key, const QVariant &value);

    ConfigPropertyMap *q;
    QPointer<KCoreConfigSkeleton> config;
    bool updatingConfigValue = false;
    bool autosave = true;
    bool notify = false;
};

ConfigPropertyMap::ConfigPropertyMap(KCoreConfigSkeleton *config, QObject *parent)
    : QQmlPropertyMap(this, parent)
    , d(new ConfigPropertyMapPrivate(this))
{
    d->config = config;

    // Reload the config only if the change signal has *not* been emitted by ourselves updating the config
    connect(config, &KCoreConfigSkeleton::configChanged, this, [this]() {
        if (!d->updatingConfigValue) {
            d->loadConfig(ConfigPropertyMapPrivate::EmitValueChanged);
        }
    });
    connect(this, &ConfigPropertyMap::valueChanged, this, [this](const QString &key, const QVariant &value) {
        d->writeConfigValue(key, value);
    });

    d->loadConfig(ConfigPropertyMapPrivate::DontEmitValueChanged);
}

ConfigPropertyMap::~ConfigPropertyMap()
{
    if (d->autosave) {
        d->writeConfig();
    }
    delete d;
}

bool ConfigPropertyMap::isAutosave() const
{
    return d->autosave;
}

void ConfigPropertyMap::setAutosave(bool autosave)
{
    d->autosave = autosave;
}

bool ConfigPropertyMap::isNotify() const
{
    return d->notify;
}

void ConfigPropertyMap::setNotify(bool notify)
{
    d->notify = notify;
}

QVariant ConfigPropertyMap::updateValue(const QString &key, const QVariant &input)
{
    Q_UNUSED(key);
    if (input.userType() == qMetaTypeId<QJSValue>()) {
        return input.value<QJSValue>().toVariant();
    }
    return input;
}

bool ConfigPropertyMap::isImmutable(const QString &key) const
{
    KConfigSkeletonItem *item = d->config.data()->findItem(key);
    if (item) {
        return item->isImmutable();
    }

    return false;
}

void ConfigPropertyMapPrivate::loadConfig(ConfigPropertyMapPrivate::LoadConfigOption option)
{
    if (!config) {
        return;
    }

    const auto &items = config.data()->items();
    for (KConfigSkeletonItem *item : items) {
        q->insert(item->key() + QStringLiteral("Default"), item->getDefault());
        q->insert(item->key(), item->property());
        if (option == EmitValueChanged) {
            Q_EMIT q->valueChanged(item->key(), item->property());
        }
    }
}

void ConfigPropertyMapPrivate::writeConfig()
{
    if (!config) {
        return;
    }

    const auto lstItems = config.data()->items();
    for (KConfigSkeletonItem *item : lstItems) {
        item->setWriteFlags(notify ? KConfigBase::Notify : KConfigBase::Normal);
        item->setProperty(q->value(item->key()));
    }

    if (autosave) {
        updatingConfigValue = true;
        config.data()->save();
        updatingConfigValue = false;
    }
}

void ConfigPropertyMapPrivate::writeConfigValue(const QString &key, const QVariant &value)
{
    KConfigSkeletonItem *item = config.data()->findItem(key);
    if (item) {
        updatingConfigValue = true;
        item->setWriteFlags(notify ? KConfigBase::Notify : KConfigBase::Normal);
        item->setProperty(value);
        if (autosave) {
            config.data()->save();
            // why read? read will update KConfigSkeletonItem::mLoadedValue,
            // allowing a write operation to be performed next time
            config.data()->read();
        }
        updatingConfigValue = false;
    }
}

}
}
