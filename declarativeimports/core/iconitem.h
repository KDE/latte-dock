/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmudnson@kde.org>
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock and is a Fork of PlasmaCore::IconItem

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ICONITEM_H
#define ICONITEM_H

// C++
#include <memory>

// Qt
#include <QQuickItem>
#include <QIcon>
#include <QImage>
#include <QPixmap>

// Plasma
#include <Plasma/Svg>

// this file is based on PlasmaCore::IconItem class, thanks to KDE
namespace Latte {
class IconItem : public QQuickItem
{
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
     * Specifies the color group to use for this icon
     * This only applies to icons loaded from the plasma theme
     */
    Q_PROPERTY(Plasma::Theme::ColorGroup colorGroup READ colorGroup WRITE setColorGroup NOTIFY colorGroupChanged)

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

    /**
     * If set, icon will try and use icons from the Plasma theme if possible
     */
    Q_PROPERTY(bool usesPlasmaTheme READ usesPlasmaTheme WRITE setUsesPlasmaTheme NOTIFY usesPlasmaThemeChanged)

    /**
     * If set, icon will provide a background and glow color
     */
    Q_PROPERTY(bool providesColors READ providesColors WRITE setProvidesColors NOTIFY providesColorsChanged)

    /**
     * Contains the last valid icon name
     */
    Q_PROPERTY(QString lastValidSourceName READ lastValidSourceName NOTIFY lastValidSourceNameChanged)

    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor glowColor READ glowColor NOTIFY glowColorChanged)
public:
    IconItem(QQuickItem *parent = nullptr);
    virtual ~IconItem();

    void setSource(const QVariant &source);
    QVariant source() const;

    void setColorGroup(Plasma::Theme::ColorGroup group);
    Plasma::Theme::ColorGroup colorGroup() const;

    void setOverlays(const QStringList &overlays);
    QStringList overlays() const;

    bool isActive() const;
    void setActive(bool active);

    void setSmooth(const bool smooth);
    bool smooth() const;

    bool isValid() const;

    bool providesColors() const;
    void setProvidesColors(const bool provides);

    bool usesPlasmaTheme() const;
    void setUsesPlasmaTheme(bool usesPlasmaTheme);

    int paintedWidth() const;
    int paintedHeight() const;

    QString lastValidSourceName();

    QColor backgroundColor() const;

    QColor glowColor() const;

    void updatePolish() Q_DECL_OVERRIDE;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override;

    void itemChange(ItemChange change, const ItemChangeData &value) override;
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    void componentComplete() Q_DECL_OVERRIDE;

signals:
    void activeChanged();
    void backgroundColorChanged();
    void colorGroupChanged();
    void glowColorChanged();
    void lastValidSourceNameChanged();
    void overlaysChanged();
    void paintedSizeChanged();
    void providesColorsChanged();
    void smoothChanged();
    void sourceChanged();
    void usesPlasmaThemeChanged();
    void validChanged();

private slots:
    void schedulePixmapUpdate();
    void enabledChanged();

private:
    void loadPixmap();
    void updateColors();
    void setLastLoadedSourceId(QString id);
    void setLastValidSourceName(QString name);
    void setBackgroundColor(QColor background);
    void setGlowColor(QColor glow);

private:
    bool m_active;
    bool m_providesColors{false};
    bool m_smooth;


    bool m_textureChanged;
    bool m_sizeChanged;
    bool m_usesPlasmaTheme;

    QColor m_backgroundColor;
    QColor m_glowColor;

    QIcon m_icon;
    QPixmap m_iconPixmap;
    QImage m_imageIcon;
    std::unique_ptr<Plasma::Svg> m_svgIcon;
    QString m_svgIconName;

    //! can be used to track changes during source "changes" independent
    //! of the source type
    int m_iconCounter{0};

    //! last source name that was valid
    QString m_lastValidSourceName;

    //! the last icon that was loaded independent of its type svg/pixmap/image
    //! this is set and used only internally. One of its uses is to check
    //! when the colors must be updated
    QString m_lastLoadedSourceId;

    //! last source name that was used in order to produce colors
    QString m_lastColorsSourceId;

    QStringList m_overlays;

    Plasma::Theme::ColorGroup m_colorGroup;

    //this contains the raw variant it was passed
    QVariant m_source;

    QSizeF m_implicitSize;
};

}
#endif // ICONITEM_H
