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

// Plasma
#include <Plasma>

namespace Plasma {
class Types;
}

namespace Latte {
namespace Layout {
Q_NAMESPACE

enum Type {
    Abstract = 0,
    Generic,
    Central,
    Shared
};
Q_ENUM_NS(Type);

}
}

namespace Latte {
namespace Layout {

class AbstractLayout : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)

    Q_PROPERTY(bool preferredForShortcutsTouched READ preferredForShortcutsTouched WRITE setPreferredForShortcutsTouched NOTIFY preferredForShortcutsTouchedChanged)

    Q_PROPERTY(QString background READ background NOTIFY backgroundChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString lastUsedActivity READ lastUsedActivity NOTIFY lastUsedActivityChanged)
    Q_PROPERTY(QString textColor READ textColor NOTIFY textColorChanged)

    Q_PROPERTY(QStringList launchers READ launchers WRITE setLaunchers NOTIFY launchersChanged)

public:
    AbstractLayout(QObject *parent, QString layoutFile, QString assignedName = QString());
    ~AbstractLayout() override;

    static const QString MultipleLayoutsName;

    int version() const;
    void setVersion(int ver);

    bool preferredForShortcutsTouched() const;
    void setPreferredForShortcutsTouched(bool touched);

    QString lastUsedActivity();
    void clearLastUsedActivity(); //!e.g. when we export a layout

    QString name() const;
    QString file() const;

    QString background() const;
    void setBackground(QString path);

    QString color() const;
    void setColor(QString color);

    QString textColor() const;
    void setTextColor(QString color);

    QStringList launchers() const;
    void setLaunchers(QStringList launcherList);

    virtual Type type() const;

// STATIC
    static QString layoutName(const QString &fileName);
    static QList<Plasma::Types::Location> combinedFreeEdges(const QList<Plasma::Types::Location> &edges1,
                                                            const QList<Plasma::Types::Location> &edges2);

signals:
    void backgroundChanged();
    void colorChanged();
    void fileChanged();
    void lastUsedActivityChanged();
    void launchersChanged();
    void nameChanged();
    void preferredForShortcutsTouchedChanged();
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
    bool m_preferredForShortcutsTouched{false};

    //if version doesn't exist it is and old layout file
    int m_version{2};

    QString m_background;
    QString m_color;
    QString m_lastUsedActivity; //the last used activity for this layout

    QString m_textColor;

    QString m_layoutFile;
    QString m_layoutName;

    QStringList m_launchers;

    KConfigGroup m_layoutGroup;
};

}
}

#endif
