/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CONFIGVIEW_H
#define CONFIGVIEW_H

#include <QQuickView>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Plasma API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Plasma {
class Applet;
}

namespace PlasmaQuick {

class ConfigViewPrivate;

class ConfigModel;

class ConfigView : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(PlasmaQuick::ConfigModel *configModel READ configModel CONSTANT)
    Q_PROPERTY(QString appletGlobalShortcut READ appletGlobalShortcut WRITE setAppletGlobalShortcut NOTIFY appletGlobalShortcutChanged)

public:
    /**
     * @param applet the applet of this ConfigView
     * @param parent the QWindow in which this ConfigView is parented to
     **/
    ConfigView(Plasma::Applet *applet, QWindow *parent = 0);
    ~ConfigView() override;

    virtual void init();

    Plasma::Applet *applet();

    QString appletGlobalShortcut() const;
    void setAppletGlobalShortcut(const QString &shortcut);

    /**
     * @return the ConfigModel of the ConfigView
     **/
    PlasmaQuick::ConfigModel *configModel() const;

Q_SIGNALS:
    void appletGlobalShortcutChanged();

protected:
    void hideEvent(QHideEvent *ev) override;
    void resizeEvent(QResizeEvent *re) override;

private:
    ConfigViewPrivate *const d;

    Q_PRIVATE_SLOT(d, void updateMinimumWidth())
    Q_PRIVATE_SLOT(d, void updateMinimumHeight())
    Q_PRIVATE_SLOT(d, void updateMaximumWidth())
    Q_PRIVATE_SLOT(d, void updateMaximumHeight())
};

}

#endif // multiple inclusion guard
