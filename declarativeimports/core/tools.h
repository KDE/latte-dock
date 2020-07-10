/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
