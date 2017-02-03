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

namespace Plasma {
class Types;
class Corona;
class Containment;
}

namespace Latte {

class DockView : public PlasmaQuick::ContainmentView {
    Q_OBJECT
    Q_PROPERTY(bool drawShadows READ drawShadows WRITE setDrawShadows NOTIFY drawShadowsChanged)

    Q_PROPERTY(int docksCount READ docksCount NOTIFY docksCountChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int maxThickness READ maxThickness WRITE setMaxThickness NOTIFY maxThicknessChanged)
    Q_PROPERTY(int normalThickness READ normalThickness WRITE setNormalThickness NOTIFY normalThicknessChanged)
    Q_PROPERTY(int shadow READ shadow WRITE setShadow NOTIFY shadowChanged)
    Q_PROPERTY(QStringList debugFlags READ debugFlags NOTIFY debugFlagsChanged)

    Q_PROPERTY(float maxLength READ maxLength WRITE setMaxLength NOTIFY maxLengthChanged)

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

    Q_PROPERTY(QRect maskArea READ maskArea WRITE setMaskArea NOTIFY maskAreaChanged)
    Q_PROPERTY(VisibilityManager *visibility READ visibility NOTIFY visibilityChanged)
    Q_PROPERTY(QQmlListProperty<QScreen> screens READ screens)

public:
    DockView(Plasma::Corona *corona, QScreen *targetScreen = nullptr);
    virtual ~DockView();

    void init();

    void adaptToScreen(QScreen *screen);

    void resizeWindow();
    void syncGeometry();

    int currentThickness() const;
    void updateAbsDockGeometry();

    int docksCount() const;

    bool drawShadows() const;
    void setDrawShadows(bool draw);

    float maxLength() const;
    void setMaxLength(float length);

    int maxThickness() const;
    void setMaxThickness(int thickness);

    int normalThickness() const;
    void setNormalThickness(int thickness);

    int shadow() const;
    void setShadow(int shadow);

    QRect maskArea() const;
    void setMaskArea(QRect area);

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    QStringList debugFlags() const;

    VisibilityManager *visibility();

    QQmlListProperty<QScreen> screens();
    static int countScreens(QQmlListProperty<QScreen> *property);
    static QScreen *atScreens(QQmlListProperty<QScreen> *property, int index);

public slots:
    Q_INVOKABLE void addNewDock();
    Q_INVOKABLE void removeDock();

    Q_INVOKABLE QList<int> freeEdges() const;
    Q_INVOKABLE QVariantList containmentActions();
    Q_INVOKABLE void setLocalDockGeometry(const QRect &geometry);
    Q_INVOKABLE bool tasksPresent();
    Q_INVOKABLE void updateEnabledBorders();

    Q_INVOKABLE void closeApplication();

protected slots:
    void showConfigurationInterface(Plasma::Applet *applet) override;

protected:
    bool event(QEvent *ev) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void addInternalViewSplitter();
    void removeInternalViewSplitter();
    void eventTriggered(QEvent *ev);

    void debugFlagsChanged();
    void dockLocationChanged();
    void docksCountChanged();
    void drawShadowsChanged();
    void enabledBordersChanged();
    void widthChanged();
    void heightChanged();
    void maxLengthChanged();
    void maxThicknessChanged();
    void normalThicknessChanged();
    void visibilityChanged();
    void maskAreaChanged();
    void shadowChanged();

    void localDockGeometryChanged();

private slots:
    void menuAboutToHide();
    void statusChanged(Plasma::Types::ItemStatus);

private:
    void addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu *desktopMenu, QEvent *event);
    void updatePosition();
    void updateFormFactor();

private:
    Plasma::Containment *containmentById(int id);

    bool m_drawShadows{false};
    int m_maxThickness{24};
    int m_normalThickness{24};
    int m_shadow{0};
    float m_maxLength{1};

    QRect m_localDockGeometry;
    QRect m_maskArea;
    QMenu *m_contextMenu;
    QPointer<PlasmaQuick::ConfigView> m_configView;
    QPointer<VisibilityManager> m_visibility;

    //only for the mask, not to actually paint
    Plasma::FrameSvg::EnabledBorders m_enabledBorders = Plasma::FrameSvg::AllBorders;
};

}

#endif
