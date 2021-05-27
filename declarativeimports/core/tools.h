/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LATTECORETOOLS_H
#define LATTECORETOOLS_H

// Qt
#include <QObject>
#include <QColor>
#include <QQmlEngine>
#include <QJSEngine>


namespace Latte{

class Tools final: public QObject
{
    Q_OBJECT

public:
    explicit Tools(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE float colorBrightness(QColor color);
    Q_INVOKABLE float colorLumina(QColor color);

private:
    float colorBrightness(QRgb rgb);
    float colorBrightness(float r, float g, float b);

    float colorLumina(QRgb rgb);
    float colorLumina(float r, float g, float b);
};

static QObject *tools_qobject_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

// NOTE: QML engine is the owner of this resource
    return new Tools;
}

}

#endif
