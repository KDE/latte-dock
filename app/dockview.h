/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef NOWDOCKVIEW_H
#define NOWDOCKVIEW_H

#include "plasmaquick/configview.h"
#include "plasmaquick/containmentview.h"
#include "visibilitymanager.h"

#include <QQuickView>
#include <QQmlListProperty>
#include <QMenu>
#include <QScreen>
#include <QPointer>
#include <QTimer>

#include <PlasmaQuick/AppletQuickItem>

namespace Plasma {
class Types;
class Corona;
class Containment;
}

namespace Latte {

class DockView : public PlasmaQuick::ContainmentView {
    Q_OBJECT
    
    Q_PROPERTY(bool compositing READ compositing NOTIFY compositingChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int length READ length WRITE setLength NOTIFY lengthChanged)
    Q_PROPERTY(int maxLength READ maxLength WRITE setMaxLength NOTIFY maxLengthChanged)
    Q_PROPERTY(int maxThickness READ maxThickness WRITE setMaxThickness NOTIFY maxThicknessChanged)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    
    Q_PROPERTY(QRect maskArea READ maskArea WRITE setMaskArea NOTIFY maskAreaChanged)
    Q_PROPERTY(VisibilityManager *visibility READ visibility NOTIFY visibilityChanged)
    Q_PROPERTY(QQmlListProperty<QScreen> screens READ screens)
    
public:
    DockView(Plasma::Corona *corona, QScreen *targetScreen = nullptr);
    virtual ~DockView();
    
    void init();
    
    // Candil::VisibilityManager *visibility();
    
    int maxThickness() const;
    void setMaxThickness(int thickness);
    
    int length() const;
    void setLength(int length);
    
    QRect maskArea() const;
    void setMaskArea(QRect area);
    
    int maxLength() const;
    void setMaxLength(int maxLength);
    
    //  Dock::Alignment alignment() const;
    //  void setAlignment(Dock::Alignment align);
    
    int offset() const;
    void setOffset(int offset);
    
    void updateOffset();
    
    VisibilityManager *visibility();
    
    bool compositing() const;
    int currentThickness() const;

    bool tasksPresent() const;
    
    void adaptToScreen(QScreen *screen);
    
    QQmlListProperty<QScreen> screens();
    static int countScreens(QQmlListProperty<QScreen> *property);
    static QScreen *atScreens(QQmlListProperty<QScreen> *property, int index);
    
public slots:
    Q_INVOKABLE void addAppletItem(QObject *item);
    Q_INVOKABLE void removeAppletItem(QObject *item);
    Q_INVOKABLE void addNewDock();
    Q_INVOKABLE QVariantList containmentActions();
    //used from the configuration window
    Q_INVOKABLE QList<int> freeEdges() const;
    Q_INVOKABLE void initialize();
    Q_INVOKABLE void removeDock();
    Q_INVOKABLE void setLocalDockGeometry(const QRect &geometry);
    Q_INVOKABLE bool tasksPresent();
    void resizeWindow();
    void restoreConfig();
    void saveConfig();
    void updateDockPosition();
    
protected slots:
    void showConfigurationInterface(Plasma::Applet *applet) override;
    
protected:
    bool event(QEvent *ev) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
//   void visibilityChanged();
    void addInternalViewSplitter();
    void eventTriggered(QEvent *ev);
    void alignmentChanged();
    void compositingChanged();
    void heightChanged();
    void lengthChanged();
    void localDockGeometryChanged();
    void maskAreaChanged();
    void maxLengthChanged();
    void maxThicknessChanged();
    void offsetChanged();
    void removeInternalViewSplitter();
    void visibilityChanged();
    void widthChanged();
    
public slots:
    void updateDockPositionSlot();
    void updateAbsDockGeometry();

private slots:
    void menuAboutToHide();
    
private:
    bool m_secondInitPass;
    
    int m_offset{0};
    int m_maxThickness{24};
    int m_length{0};
    int m_maxLength{INT_MAX};
    
    QRect m_localDockGeometry;
    QRect m_maskArea;
    QPointer<PlasmaQuick::ConfigView> m_configView;
    QMenu *m_contextMenu;

    QList<PlasmaQuick::AppletQuickItem *> m_appletItems;
    
    QTimer m_timerGeometry;
    QTimer m_lockGeometry;
    Plasma::Theme *theme{nullptr};
    
    QPointer<VisibilityManager> m_visibility;
    
    void initWindow();

    void addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu *desktopMenu, QEvent *event);
};

}

#endif
