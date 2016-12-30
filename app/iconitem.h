/*
*   Copyright 2012 Marco Martin <mart@kde.org>
*   Copyright 2014 David Edmundson <davidedmudnson@kde.org>
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

#ifndef ICONITEM_H
#define ICONITEM_H

#include <memory>

#include <QQuickItem>
#include <QIcon>
#include <QImage>
#include <QPixmap>

#include <Plasma/Svg>

// this file is based on PlasmaCore::IconItem class, thanks to KDE
namespace Latte {
class IconItem : public QQuickItem {
    Q_OBJECT
    
    /**
     * Sets the icon to be displayed. Source can be one of:
     *  - iconName (as a string)
     *  - URL
     *  - QImage
     *  - QPixmap
     *  - QIcon
     *
     * When passing an icon name (or a QIcon with an icon name set) it will:
     *  - load the plasma variant if usesPlasmaTheme is set and exists
     *  - otherwise try to load the icon as an SVG so colorscopes apply
     *  - load the icon as normal
     */
    Q_PROPERTY(QVariant source READ source WRITE setSource NOTIFY sourceChanged)
    
    /**
      * Specifies the overlay(s) for this icon
      */
    Q_PROPERTY(QStringList overlays READ overlays WRITE setOverlays NOTIFY overlaysChanged)
    
    /**
     * See QQuickItem::smooth
     */
    Q_PROPERTY(bool smooth READ smooth WRITE setSmooth NOTIFY smoothChanged)
    
    /**
     * Apply a visual indication that this icon is active.
     * Typically used to indicate that it is hovered
     */
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    
    /**
     * True if a valid icon is set. False otherwise.
     */
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    
    /**
     * The width of the icon that is actually painted
     */
    Q_PROPERTY(int paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    
    /**
     * The height of the icon actually being drawn.
     */
    Q_PROPERTY(int paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)
    
public:
    IconItem(QQuickItem *parent = nullptr);
    virtual ~IconItem();
    
    void setSource(const QVariant &source);
    QVariant source() const;
    
    void setOverlays(const QStringList &overlays);
    QStringList overlays() const;
    
    bool isActive() const;
    void setActive(bool active);
    
    void setSmooth(const bool smooth);
    bool smooth() const;
    
    bool isValid() const;
    
    int paintedWidth() const;
    int paintedHeight() const;
    
    void updatePolish() Q_DECL_OVERRIDE;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override;
    
    void itemChange(ItemChange change, const ItemChangeData &value) override;
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
                         
    void componentComplete() Q_DECL_OVERRIDE;
    
signals:
    void overlaysChanged();
    void activeChanged();
    void sourceChanged();
    void smoothChanged();
    void validChanged();
    void paintedSizeChanged();
    
private slots:
    void schedulePixmapUpdate();
    void enabledChanged();
    
private:
    void loadPixmap();
    
    QIcon m_icon;
    QPixmap m_iconPixmap;
    QImage m_imageIcon;
    std::unique_ptr<Plasma::Svg> m_svgIcon;
    QString m_svgIconName;
    QStringList m_overlays;
    //this contains the raw variant it was passed
    QVariant m_source;
    
    QSizeF m_implicitSize;
    
    bool m_smooth;
    bool m_active;
    
    bool m_textureChanged;
    bool m_sizeChanged;
    
    
};
}
#endif // ICONITEM_H
