/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLASMAQUICKCONTAINMENTVIEW_H
#define PLASMAQUICKCONTAINMENTVIEW_H

#include <kquickaddons/quickviewsharedengine.h>

#include "plasma/corona.h"
#include "plasma/containment.h"

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

namespace PlasmaQuick {

class ContainmentViewPrivate;

class ContainmentView : public KQuickAddons::QuickViewSharedEngine
{
    Q_OBJECT
    Q_PROPERTY(Plasma::Types::Location location READ location WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(Plasma::Types::FormFactor formFactor READ formFactor NOTIFY formFactorChanged)
    Q_PROPERTY(QRectF screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

public:
    /**
     * @param corona the corona of this view
     * @param parent the QWindow this ContainmentView is parented to
     **/
    explicit ContainmentView(Plasma::Corona *corona, QWindow *parent = 0);
    ~ContainmentView() override;

    /**
     * @return the corona of this view
     **/
    Plasma::Corona *corona() const;

    /**
     * @return the KConfigGroup of this view
     **/
    virtual KConfigGroup config() const;

    /**
     * sets the containment for this view
     * @param cont the containment of this view
     **/
    void setContainment(Plasma::Containment *cont);

    /**
     * @return the containment of this ContainmentView
     **/
    Plasma::Containment *containment() const;

    /**
     * @return the location of this ContainmentView
     **/
    Plasma::Types::Location location() const;

    /**
     * Sets the location of the ContainmentView
     * @param location the location of the ContainmentView
     **/
    void setLocation(Plasma::Types::Location location);

    /**
     * @return the formfactor of the ContainmentView
     **/
    Plasma::Types::FormFactor formFactor() const;

    /**
     * @return the screenGeometry of the ContainmentView
     **/
    QRectF screenGeometry();

protected Q_SLOTS:
    /**
     * It will be called when the configuration is requested
     */
    virtual void showConfigurationInterface(Plasma::Applet *applet);

Q_SIGNALS:
    /**
     * emitted when the location is changed
     **/
    void locationChanged(Plasma::Types::Location location);

    /**
     * emitted when the formfactor is changed
     **/
    void formFactorChanged(Plasma::Types::FormFactor formFactor);

    /**
     * emitted when the containment is changed
     **/
    void containmentChanged();

    /**
     * emitted when the screenGeometry is changed
     **/
    void screenGeometryChanged();

private:
    ContainmentViewPrivate *const d;
    Q_PRIVATE_SLOT(d, void updateDestroyed(bool))
    friend class ContainmentViewPrivate;
};

}

#endif // CONTAINMENTVIEW_H
