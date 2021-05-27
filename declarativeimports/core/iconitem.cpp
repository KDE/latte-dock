/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmudnson@kde.org>
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock and is a Fork of PlasmaCore::IconItem

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iconitem.h"

// local
#include "extras.h"

// Qt
#include <QDebug>
#include <QPainter>
#include <QPaintEngine>
#include <QQuickWindow>
#include <QPixmap>
#include <QSGSimpleTextureNode>
#include <QuickAddons/ManagedTextureNode>
#include <QLatin1String>

// KDE
#include <KIconTheme>
#include <KIconThemes/KIconLoader>
#include <KIconThemes/KIconEffect>

namespace Latte {

IconItem::IconItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_active(false),
      m_smooth(false),
      m_textureChanged(false),
      m_sizeChanged(false),
      m_usesPlasmaTheme(false),
      m_lastValidSourceName(QString()),
      m_colorGroup(Plasma::Theme::NormalColorGroup)
{
    setFlag(ItemHasContents, true);
    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()),
            this, SIGNAL(implicitWidthChanged()));
    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()),
            this, SIGNAL(implicitHeightChanged()));
    connect(this, &QQuickItem::enabledChanged,
            this, &IconItem::enabledChanged);
    connect(this, &QQuickItem::windowChanged,
            this, &IconItem::schedulePixmapUpdate);
    connect(this, SIGNAL(overlaysChanged()),
            this, SLOT(schedulePixmapUpdate()));
    connect(this, SIGNAL(providesColorsChanged()),
            this, SLOT(schedulePixmapUpdate()));

    //initialize implicit size to the Dialog size
    setImplicitWidth(KIconLoader::global()->currentSize(KIconLoader::Dialog));
    setImplicitHeight(KIconLoader::global()->currentSize(KIconLoader::Dialog));
    setSmooth(true);
}

IconItem::~IconItem()
{
}

void IconItem::setSource(const QVariant &source)
{
    if (source == m_source) {
        return;
    }

    m_source = source;
    QString sourceString = source.toString();

    // If the QIcon was created with QIcon::fromTheme(), try to load it as svg
    if (source.canConvert<QIcon>() && !source.value<QIcon>().name().isEmpty()) {
        sourceString = source.value<QIcon>().name();
    }

    if (!sourceString.isEmpty()) {
        setLastValidSourceName(sourceString);
        setLastLoadedSourceId(sourceString);

        //If a url in the form file:// is passed, take the image pointed by that from disk
        QUrl url(sourceString);

        if (url.isLocalFile()) {
            m_icon = QIcon();
            m_imageIcon = QImage(url.path());
            m_svgIconName.clear();
            m_svgIcon.reset();
        } else {
            if (!m_svgIcon) {
                m_svgIcon = std::make_unique<Plasma::Svg>(this);
                m_svgIcon->setColorGroup(m_colorGroup);
                m_svgIcon->setStatus(Plasma::Svg::Normal);
                m_svgIcon->setUsingRenderingCache(false);
                m_svgIcon->setDevicePixelRatio((window() ? window()->devicePixelRatio() : qApp->devicePixelRatio()));
                connect(m_svgIcon.get(), &Plasma::Svg::repaintNeeded, this, &IconItem::schedulePixmapUpdate);
            }

            if (m_usesPlasmaTheme) {
                //try as a svg icon from plasma theme
                m_svgIcon->setImagePath(QLatin1String("icons/") + sourceString.split('-').first());
                m_svgIcon->setContainsMultipleImages(true);
                //invalidate the image path to recalculate it later
            } else {
                m_svgIcon->setImagePath(QString());
            }

            //success?
            if (m_svgIcon->isValid() && m_svgIcon->hasElement(sourceString)) {
                m_icon = QIcon();
                m_svgIconName = sourceString;
                //ok, svg not available from the plasma theme
            } else {
                //try to load from iconloader an svg with Plasma::Svg
                const auto *iconTheme = KIconLoader::global()->theme();
                QString iconPath;

                if (iconTheme) {
                    iconPath = iconTheme->iconPath(sourceString + QLatin1String(".svg")
                                                   , static_cast<int>(qMin(width(), height()))
                                                   , KIconLoader::MatchBest);

                    if (iconPath.isEmpty()) {
                        iconPath = iconTheme->iconPath(sourceString + QLatin1String(".svgz")
                                                       , static_cast<int>(qMin(width(), height()))
                                                       , KIconLoader::MatchBest);
                    }
                } else {
                    qWarning() << "KIconLoader has no theme set";
                }

                if (!iconPath.isEmpty()) {
                    m_svgIcon->setImagePath(iconPath);
                    m_svgIconName = sourceString;
                    //fail, use QIcon
                } else {
                    //if we started with a QIcon use that.
                    m_icon = source.value<QIcon>();

                    if (m_icon.isNull()) {
                        m_icon = QIcon::fromTheme(sourceString);
                    }

                    m_svgIconName.clear();
                    m_svgIcon.reset();
                    m_imageIcon = QImage();
                }
            }
        }
    } else if (source.canConvert<QIcon>()) {
        m_icon = source.value<QIcon>();
        m_iconCounter++;
        setLastLoadedSourceId("_icon_"+QString::number(m_iconCounter));

        m_imageIcon = QImage();
        m_svgIconName.clear();
        m_svgIcon.reset();
    } else if (source.canConvert<QImage>()) {
        m_imageIcon = source.value<QImage>();
        m_iconCounter++;
        setLastLoadedSourceId("_image_"+QString::number(m_iconCounter));

        m_icon = QIcon();
        m_svgIconName.clear();
        m_svgIcon.reset();
    } else {
        m_icon = QIcon();
        m_imageIcon = QImage();
        m_svgIconName.clear();
        m_svgIcon.reset();
    }

    if (width() > 0 && height() > 0) {
        schedulePixmapUpdate();
    }

    emit sourceChanged();
    emit validChanged();
}

QVariant IconItem::source() const
{
    return m_source;
}

void IconItem::setLastLoadedSourceId(QString id)
{
    if (m_lastLoadedSourceId == id) {
        return;
    }

    m_lastLoadedSourceId = id;
}

QString IconItem::lastValidSourceName()
{
    return m_lastValidSourceName;
}

void IconItem::setLastValidSourceName(QString name)
{
    if (m_lastValidSourceName == name || name.isEmpty() || name == QLatin1String("application-x-executable")) {
        return;
    }

    m_lastValidSourceName = name;

    emit lastValidSourceNameChanged();
}

void IconItem::setColorGroup(Plasma::Theme::ColorGroup group)
{
    if (m_colorGroup == group) {
        return;
    }

    m_colorGroup = group;

    if (m_svgIcon) {
        m_svgIcon->setColorGroup(group);
    }

    emit colorGroupChanged();
}

Plasma::Theme::ColorGroup IconItem::colorGroup() const
{
    return m_colorGroup;
}


void IconItem::setOverlays(const QStringList &overlays)
{
    if (overlays == m_overlays) {
        return;
    }

    m_overlays = overlays;
    emit overlaysChanged();
}

QStringList IconItem::overlays() const
{
    return m_overlays;
}


bool IconItem::isActive() const
{
    return m_active;
}

void IconItem::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;

    if (isComponentComplete()) {
        schedulePixmapUpdate();
    }

    emit activeChanged();
}

bool IconItem::providesColors() const
{
    return m_providesColors;
}

void IconItem::setProvidesColors(const bool provides)
{
    if (m_providesColors == provides) {
        return;
    }

    m_providesColors = provides;
    emit providesColorsChanged();
}

void IconItem::setSmooth(const bool smooth)
{
    if (smooth == m_smooth) {
        return;
    }

    m_smooth = smooth;
    update();
}

bool IconItem::smooth() const
{
    return m_smooth;
}

bool IconItem::isValid() const
{
    return !m_icon.isNull() || m_svgIcon || !m_imageIcon.isNull();
}

int IconItem::paintedWidth() const
{
    return boundingRect().size().toSize().width();
}

int IconItem::paintedHeight() const
{
    return boundingRect().size().toSize().height();
}

bool IconItem::usesPlasmaTheme() const
{
    return m_usesPlasmaTheme;
}

void IconItem::setUsesPlasmaTheme(bool usesPlasmaTheme)
{
    if (m_usesPlasmaTheme == usesPlasmaTheme) {
        return;
    }

    m_usesPlasmaTheme = usesPlasmaTheme;

    // Reload icon with new settings
    const QVariant src = m_source;
    m_source.clear();
    setSource(src);

    update();
    emit usesPlasmaThemeChanged();
}

void IconItem::updatePolish()
{
    QQuickItem::updatePolish();
    loadPixmap();
}

QSGNode *IconItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    if (m_iconPixmap.isNull() || width() < 1.0 || height() < 1.0) {
        delete oldNode;
        return nullptr;
    }

    ManagedTextureNode *textureNode = dynamic_cast<ManagedTextureNode *>(oldNode);

    if (!textureNode || m_textureChanged) {
        if (oldNode)
            delete oldNode;

        textureNode = new ManagedTextureNode;
        textureNode->setTexture(QSharedPointer<QSGTexture>(window()->createTextureFromImage(m_iconPixmap.toImage(), QQuickWindow::TextureCanUseAtlas)));
        textureNode->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);

        m_sizeChanged = true;
        m_textureChanged = false;
    }

    if (m_sizeChanged) {
        const auto iconSize = qMin(boundingRect().size().width(), boundingRect().size().height());
        const QRectF destRect(QPointF(boundingRect().center() - QPointF(iconSize / 2, iconSize / 2)), QSizeF(iconSize, iconSize));
        textureNode->setRect(destRect);
        m_sizeChanged = false;
    }

    return textureNode;
}

void IconItem::schedulePixmapUpdate()
{
    polish();
}

void IconItem::enabledChanged()
{
    schedulePixmapUpdate();
}

QColor IconItem::backgroundColor() const
{
    return m_backgroundColor;
}

void IconItem::setBackgroundColor(QColor background)
{
    if (m_backgroundColor == background) {
        return;
    }

    m_backgroundColor = background;
    emit backgroundColorChanged();
}

QColor IconItem::glowColor() const
{
    return m_glowColor;
}

void IconItem::setGlowColor(QColor glow)
{
    if (m_glowColor == glow) {
        return;
    }

    m_glowColor = glow;
    emit glowColorChanged();
}

void IconItem::updateColors()
{
    QImage icon = m_iconPixmap.toImage();

    if (icon.format() != QImage::Format_Invalid) {
        float rtotal = 0, gtotal = 0, btotal = 0;
        float total = 0.0f;

        for(int row=0; row<icon.height(); ++row) {
            QRgb *line = (QRgb *)icon.scanLine(row);

            for(int col=0; col<icon.width(); ++col) {
                QRgb pix = line[col];

                int r = qRed(pix);
                int g = qGreen(pix);
                int b = qBlue(pix);
                int a = qAlpha(pix);

                float saturation = (qMax(r, qMax(g, b)) - qMin(r, qMin(g, b))) / 255.0f;
                float relevance = .1 + .9 * (a / 255.0f) * saturation;

                rtotal += (float)(r * relevance);
                gtotal += (float)(g * relevance);
                btotal += (float)(b * relevance);

                total += relevance * 255;
            }
        }

        int nr = (rtotal / total) * 255;
        int ng = (gtotal / total) * 255;
        int nb = (btotal / total) * 255;

        QColor tempColor(nr, ng, nb);

        if (tempColor.hsvSaturationF() > 0.15f) {
            tempColor.setHsvF(tempColor.hueF(), 0.65f, tempColor.valueF());
        }

        tempColor.setHsvF(tempColor.hueF(), tempColor.saturationF(), 0.55f); //original 0.90f ???

        setBackgroundColor(tempColor);

        tempColor.setHsvF(tempColor.hueF(), tempColor.saturationF(), 1.0f);

        setGlowColor(tempColor);
    }
}

void IconItem::loadPixmap()
{
    if (!isComponentComplete()) {
        return;
    }

    const auto size = qMin(width(), height());
    //final pixmap to paint
    QPixmap result;

    if (size <= 0) {
        m_iconPixmap = QPixmap();
        update();
        return;
    } else if (m_svgIcon) {
        m_svgIcon->resize(size, size);

        if (m_svgIcon->hasElement(m_svgIconName)) {
            result = m_svgIcon->pixmap(m_svgIconName);
        } else if (!m_svgIconName.isEmpty()) {
            const auto *iconTheme = KIconLoader::global()->theme();
            QString iconPath;

            if (iconTheme) {
                iconPath = iconTheme->iconPath(m_svgIconName + QLatin1String(".svg")
                                               , static_cast<int>(qMin(width(), height()))
                                               , KIconLoader::MatchBest);

                if (iconPath.isEmpty()) {
                    iconPath = iconTheme->iconPath(m_svgIconName + QLatin1String(".svgz"),
                                                   static_cast<int>(qMin(width(), height()))
                                                   , KIconLoader::MatchBest);
                }
            } else {
                qWarning() << "KIconLoader has no theme set";
            }

            if (!iconPath.isEmpty()) {
                m_svgIcon->setImagePath(iconPath);
            }

            result = m_svgIcon->pixmap();
        }
    } else if (!m_icon.isNull()) {
        result = m_icon.pixmap(QSize(static_cast<int>(size), static_cast<int>(size))
                               * (window() ? window()->devicePixelRatio() : qApp->devicePixelRatio()));
    } else if (!m_imageIcon.isNull()) {
        result = QPixmap::fromImage(m_imageIcon);
    } else {
        m_iconPixmap = QPixmap();
        update();
        return;
    }

    // Strangely KFileItem::overlays() returns empty string-values, so
    // we need to check first whether an overlay must be drawn at all.
    // It is more efficient to do it here, as KIconLoader::drawOverlays()
    // assumes that an overlay will be drawn and has some additional
    // setup time.
    for (const QString &overlay : m_overlays) {
        if (!overlay.isEmpty()) {
            // There is at least one overlay, draw all overlays above m_pixmap
            // and cancel the check
            KIconLoader::global()->drawOverlays(m_overlays, result, KIconLoader::Desktop);
            break;
        }
    }

    if (!isEnabled()) {
        result = KIconLoader::global()->iconEffect()->apply(result, KIconLoader::Desktop, KIconLoader::DisabledState);
    } else if (m_active) {
        result = KIconLoader::global()->iconEffect()->apply(result, KIconLoader::Desktop, KIconLoader::ActiveState);
    }

    m_iconPixmap = result;

    if (m_providesColors && m_lastLoadedSourceId != m_lastColorsSourceId) {
        m_lastColorsSourceId = m_lastLoadedSourceId;
        updateColors();
    }

    m_textureChanged = true;
    //don't animate initial setting
    update();
}

void IconItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
}

void IconItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size()) {
        m_sizeChanged = true;

        if (newGeometry.width() > 1 && newGeometry.height() > 1) {
            schedulePixmapUpdate();
        } else {
            update();
        }

        const auto oldSize = qMin(oldGeometry.size().width(), oldGeometry.size().height());
        const auto newSize = qMin(newGeometry.size().width(), newGeometry.size().height());

        if (!almost_equal(oldSize, newSize, 2)) {
            emit paintedSizeChanged();
        }
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

void IconItem::componentComplete()
{
    QQuickItem::componentComplete();
    schedulePixmapUpdate();
}

}
