/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ABSTRACTLAYOUT_H
#define ABSTRACTLAYOUT_H

// Qt
#include <QObject>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

namespace Latte {
namespace Layout {

class AbstractLayout : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)

    Q_PROPERTY(QString background READ background NOTIFY backgroundChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString textColor READ textColor NOTIFY textColorChanged)

public:
    AbstractLayout(QObject *parent, QString layoutFile, QString assignedName = QString());
    ~AbstractLayout() override;

    int version() const;
    void setVersion(int ver);

    QString name() const;
    QString file() const;

    QString background() const;
    void setBackground(QString path);

    QString color() const;
    void setColor(QString color);

    QString textColor() const;
    void setTextColor(QString color);

// STATIC
    static QString layoutName(const QString &fileName);

signals:
    void backgroundChanged();
    void colorChanged();
    void fileChanged();
    void nameChanged();
    void textColorChanged();
    void versionChanged();

protected slots:
    void loadConfig();
    void saveConfig();

protected:
    void init();
    void setName(QString name);
    void setFile(QString file);

protected:
    bool m_loadedCorrectly{false};

    //if version doesn't exist it is and old layout file
    int m_version{2};

    QString m_background;
    QString m_color;
    QString m_textColor;

    QString m_layoutFile;
    QString m_layoutName;

    KConfigGroup m_layoutGroup;
};

}
}

#endif
