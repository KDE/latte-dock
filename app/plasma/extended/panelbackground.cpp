/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "panelbackground.h"

// local
#include "theme.h"

// Qt
#include <QDebug>
#include <QImage>

#define CENTERWIDTH 100
#define CENTERHEIGHT 50

#define BASELINESHADOWTHRESHOLD 5

namespace Latte {
namespace PlasmaExtended {

PanelBackground::PanelBackground(Plasma::Types::Location edge, Theme *parent)
    : QObject(parent),
      m_location(edge),
      m_parentTheme(parent)
{
}

PanelBackground::~PanelBackground()
{
}

bool PanelBackground::hasMask(Plasma::Svg *svg) const
{
    if (!svg) {
        return false;
    }

    return svg->hasElement("mask-topleft");
}

int PanelBackground::paddingTop() const
{
    return m_paddingTop;
}

int PanelBackground::paddingLeft() const
{
    return m_paddingLeft;
}

int PanelBackground::paddingBottom() const
{
    return m_paddingBottom;
}

int PanelBackground::paddingRight() const
{
    return m_paddingRight;
}

int PanelBackground::roundness() const
{
    return m_roundness;
}

int PanelBackground::shadowSize() const
{
    return m_shadowSize;
}

float PanelBackground::maxOpacity() const
{
    return m_maxOpacity;
}

QColor PanelBackground::shadowColor() const
{
    return m_shadowColor;
}

QString PanelBackground::prefixed(const QString &id)
{
    if (m_location == Plasma::Types::TopEdge) {
        return QString("north-"+id);
    } else if (m_location == Plasma::Types::LeftEdge) {
        return QString("west-"+id);
    } else if (m_location == Plasma::Types::BottomEdge) {
        return QString("south-"+id);
    } else if (m_location == Plasma::Types::RightEdge) {
        return QString("east-"+id);
    }

    return id;
}

QString PanelBackground::element(Plasma::Svg *svg, const QString &id)
{
    if (!svg) {
        return "";
    }

    if (svg->hasElement(prefixed(id))) {
        return prefixed(id);
    }

    if (svg->hasElement(id)) {
        return id;
    }

    return "";
}

void PanelBackground::updateMaxOpacity(Plasma::Svg *svg)
{
    if (!svg) {
        return;
    }

    QImage center = svg->image(QSize(CENTERWIDTH, CENTERHEIGHT), element(svg, "center"));

    float alphasum{0};

    //! calculating the mid opacity (this is needed in order to handle Oxygen
    //! that has different opacity levels in the same center element)
    for (int row=0; row<2; ++row) {
        QRgba64 *line = (QRgba64 *)center.scanLine(row);

        for (int col=0; col<CENTERWIDTH; ++col) {
            QRgba64 pixelData = line[col];
            alphasum += ((float)qAlpha(pixelData)/(float)255);
        }
    }

    m_maxOpacity = alphasum / (float)(2 * CENTERWIDTH);

    emit maxOpacityChanged();
}

void PanelBackground::updatePaddings(Plasma::Svg *svg)
{
    if (!svg) {
        return;
    }

    m_paddingTop = svg->elementSize(element(svg, "top")).height();
    m_paddingLeft = svg->elementSize(element(svg, "left")).width();
    m_paddingBottom = svg->elementSize(element(svg, "bottom")).height();
    m_paddingRight = svg->elementSize(element(svg, "right")).width();

    emit paddingsChanged();
}

void PanelBackground::updateRoundnessFromMask(Plasma::Svg *svg)
{
    if (!svg) {
        return;
    }

    bool topLeftCorner = (m_location == Plasma::Types::BottomEdge || m_location == Plasma::Types::RightEdge);

    QString cornerId = (topLeftCorner ? "mask-topleft" : "mask-bottomright");
    QImage corner = svg->image(svg->elementSize(cornerId), cornerId);

    int baseRow = (topLeftCorner ? corner.height()-1 : 0);
    int baseCol = (topLeftCorner ? corner.width()-1 : 0);

    int baseLineLength = 0;
    int roundnessLines = 0;

    if (topLeftCorner) {
        //! TOPLEFT corner
        QRgba64 *line = (QRgba64 *)corner.scanLine(baseRow);
        QRgba64 basePoint = line[baseCol];

        QRgba64 *isRoundedLine = (QRgba64 *)corner.scanLine(0);
        QRgba64 isRoundedPoint = isRoundedLine[0];

        //! If there is roundness, if that point is not fully transparent then
        //! there is no roundness
        if (qAlpha(isRoundedPoint) == 0) {

            if (qAlpha(basePoint) > 0) {
                //! calculate the mask baseLine length
                for(int c = baseCol; c>=0; --c) {
                    QRgba64 *l = (QRgba64 *)corner.scanLine(baseRow);
                    QRgba64 point = line[c];

                    if (qAlpha(point) > 0) {
                        baseLineLength ++;
                    } else {
                        break;
                    }
                }
            }

            qDebug() << " TOP LEFT CORNER MASK base line length :: " << baseLineLength;

            if (baseLineLength>0) {
                int headLimitR = baseRow;
                int tailLimitR = baseRow;

                for (int r = baseRow-1; r>=0; --r) {
                    QRgba64 *line = (QRgba64 *)corner.scanLine(r);
                    QRgba64 fpoint = line[baseCol];
                    if (qAlpha(fpoint) == 0) {
                        //! a line that is not part of the roundness because its first pixel is fully transparent
                        break;
                    }

                    headLimitR = r;
                }

                int c = qMax(0, corner.width() - baseLineLength);

                for (int r = baseRow-1; r>=0; --r) {
                    QRgba64 *line = (QRgba64 *)corner.scanLine(r);
                    QRgba64 point = line[c];

                    if (qAlpha(point) != 255) {
                        tailLimitR = r;
                        break;
                    }
                }

                //qDebug() << "   -> calculations: " << ", tail row :" <<  tailLimitR << " | head row: " << headLimitR;

                if (headLimitR != tailLimitR) {
                    roundnessLines = tailLimitR - headLimitR + 1;
                }
            }
        }
    } else {
        //! BOTTOMRIGHT CORNER
        //! it should be TOPRIGHT corner in that case
        QRgba64 *line = (QRgba64 *)corner.scanLine(baseRow);
        QRgba64 basePoint = line[baseCol];

        QRgba64 *isRoundedLine = (QRgba64 *)corner.scanLine(corner.height()-1);
        QRgba64 isRoundedPoint = isRoundedLine[corner.width()-1];

        //! If there is roundness, if that point is not fully transparent then
        //! there is no roundness
        if (qAlpha(isRoundedPoint) == 0) {

            if (qAlpha(basePoint) > 0) {
                //! calculate the mask baseLine length
                for(int c = baseCol; c<corner.width(); ++c) {
                    QRgba64 *l = (QRgba64 *)corner.scanLine(baseRow);
                    QRgba64 point = line[c];

                    if (qAlpha(point) > 0) {
                        baseLineLength ++;
                    } else {
                        break;
                    }
                }
            }

            qDebug() << " BOTTOM RIGHT CORNER MASK base line length :: " << baseLineLength;

            if (baseLineLength>0) {
                int headLimitR = 0;
                int tailLimitR = 0;

                for (int r = baseRow+1; r<=corner.height(); ++r) {
                    QRgba64 *line = (QRgba64 *)corner.scanLine(r);
                    QRgba64 fpoint = line[baseCol];
                    if (qAlpha(fpoint) == 0) {
                        //! a line that is not part of the roundness because its first pixel is not trasparent
                        break;
                    }

                    headLimitR = r;
                }

                int c = baseLineLength - 1;

                for (int r = baseRow+1; r<=corner.height(); ++r) {
                    QRgba64 *line = (QRgba64 *)corner.scanLine(r);
                    QRgba64 point = line[c];

                    if (qAlpha(point) != 255) {
                        tailLimitR = r;
                        break;
                    }
                }

                //qDebug() << "   -> calculations: " << ", tail row :" <<  tailLimitR << " | head row: " << headLimitR;

                if (headLimitR != tailLimitR) {
                    roundnessLines = headLimitR - tailLimitR + 1;
                }
            }
        }
    }

    m_roundness = roundnessLines;
    emit roundnessChanged();
}



void PanelBackground::updateRoundnessFromShadows(Plasma::Svg *svg)
{
    //! 1.  Algorithm is choosing which corner shadow based on panel location
    //! 2.  For that corner discovers the maxOpacity (most solid shadow point) and
    //!     how pixels (distance) is to the most solid point, that is called [baseLineLength]
    //! 3.  After [2] the algorigthm for each next line calculates the maxOpacity
    //!     for that line and how many points are needed to reach there. If the points
    //!     to reach the line max opacity are shorter than baseLineLength then that line
    //!     is considered part of the roundness
    //! 3.1 Avoid zig-zag cases such as the Air plasma theme case. When the shadow is not
    //!     following a straight line until reaching the rounded part the algorithm is
    //!     considering as valid roundness only the last part of the discovered roundness and
    //!     ignores all the previous.
    //! 4.  Calculating the lines that are shorter than the baseline provides
    //!     the discovered roundness

    if (!svg) {
        return;
    }

    bool topLeftCorner = (m_location == Plasma::Types::BottomEdge || m_location == Plasma::Types::RightEdge);

    QString cornerId = (topLeftCorner ? "shadow-topleft" : "shadow-bottomright");
    QImage corner = svg->image(svg->elementSize(cornerId), cornerId);

    int baseRow = (topLeftCorner ? corner.height()-1 : 0);
    int baseCol = (topLeftCorner ? corner.width()-1 : 0);

    int baseLineLength = 0;
    int roundnessLines = 0;

    if (topLeftCorner) {
        //! TOPLEFT corner
        QRgba64 *line = (QRgba64 *)corner.scanLine(baseRow);
        QRgba64 basePoint = line[baseCol];

        int baseShadowMaxOpacity = 0;

        if (qAlpha(basePoint) == 0) {
            //! calculate the shadow maxOpacity in the base line
            //! and number of pixels to reach there
            for(int c = baseCol; c>=0; --c) {
                QRgba64 *l = (QRgba64 *)corner.scanLine(baseRow);
                QRgba64 point = line[c];

                if (qAlpha(point) > baseShadowMaxOpacity) {
                    baseShadowMaxOpacity = qAlpha(point);
                    baseLineLength = (baseCol - c + 1);
                }
            }
        }

        qDebug() << " TOP LEFT CORNER SHADOW base line length :: " << baseLineLength << " with max shadow opacity : " << baseShadowMaxOpacity;

        if (baseLineLength>0) {
            for (int r = baseRow-1; r>=0; --r) {
                QRgba64 *line = (QRgba64 *)corner.scanLine(r);
                QRgba64 fpoint = line[baseCol];
                if (qAlpha(fpoint) != 0) {
                    //! a line that is not part of the roundness because its first pixel is not trasparent
                    break;
                }

                int transPixels = 0;
                int rowMaxOpacity = 0;

                for(int c = baseCol; c>=0; --c) {
                    QRgba64 *l = (QRgba64 *)corner.scanLine(r);
                    QRgba64 point = line[c];

                    if (qAlpha(point) > rowMaxOpacity) {
                        rowMaxOpacity = qAlpha(point);
                        continue;
                    }
                }

                for(int c = baseCol; c>=(baseCol - baseLineLength + 1); --c) {
                    QRgba64 *l = (QRgba64 *)corner.scanLine(r);
                    QRgba64 point = line[c];

                    if (qAlpha(point) != rowMaxOpacity) {
                        transPixels++;
                        continue;
                    }

                    if (transPixels != baseLineLength) {
                        roundnessLines++;
                        break;
                    }
                }

                if (transPixels == baseLineLength) {
                    //! 3.1 avoid zig-zag shadows Air plasma theme case
                    roundnessLines = 0;
                }

                //qDebug() << "    -> line: " << r << ", low transparency pixels :" << transPixels << " | " << " rowMaxOpacity :"<< rowMaxOpacity << ", " << (transPixels != baseLineLength);
            }
        }
    } else {
        //! BOTTOMRIGHT CORNER
        //! it should be TOPRIGHT corner in that case
        QRgba64 *line = (QRgba64 *)corner.scanLine(baseRow);
        QRgba64 basePoint = line[baseCol];

        int baseShadowMaxOpacity = 0;

        if (qAlpha(basePoint) == 0) {
            //! calculate the base line transparent pixels
            for(int c = baseCol; c<corner.width(); ++c) {
                QRgba64 *l = (QRgba64 *)corner.scanLine(baseRow);
                QRgba64 point = line[c];

                if (qAlpha(point) > baseShadowMaxOpacity) {
                    baseShadowMaxOpacity = qAlpha(point);
                    baseLineLength = c + 1;
                }
            }
        }

        qDebug() << " BOTTOM RIGHT CORNER SHADOW base line length :: " << baseLineLength << " with max shadow opacity : " << baseShadowMaxOpacity;

        if (baseLineLength>0) {
            for (int r = baseRow+1; r<=corner.height(); ++r) {
                QRgba64 *line = (QRgba64 *)corner.scanLine(r);
                QRgba64 fpoint = line[baseCol];
                if (qAlpha(fpoint) != 0) {
                    //! a line that is not part of the roundness because its first pixel is not trasparent
                    break;
                }

                int transPixels = 0;
                int rowMaxOpacity = 0;

                for(int c = baseCol; c<corner.width(); ++c) {
                    QRgba64 *l = (QRgba64 *)corner.scanLine(r);
                    QRgba64 point = line[c];

                    if (qAlpha(point) > rowMaxOpacity) {
                        rowMaxOpacity = qAlpha(point);
                        baseLineLength = c + 1;
                    }
                }

                for(int c = baseCol; c<baseLineLength; ++c) {
                    QRgba64 *l = (QRgba64 *)corner.scanLine(r);
                    QRgba64 point = line[c];

                    if (qAlpha(point) != rowMaxOpacity) {
                        transPixels++;
                        continue;
                    }

                    if (transPixels != baseLineLength) {
                        roundnessLines++;
                        break;
                    }
                }

                if (transPixels == baseLineLength) {
                    //! 3.1 avoid zig-zag shadows Air plasma theme case
                    roundnessLines = 0;
                }

                //qDebug() << "    -> line: " << r << ", low transparency pixels :" << transPixels << " | " << " rowMaxOpacity :"<< rowMaxOpacity << ", " << (transPixels != baseLineLength);
            }
        }
    }

    m_roundness = roundnessLines;
    emit roundnessChanged();
}

void PanelBackground::updateRoundnessFallback(Plasma::Svg *svg)
{
    if (!svg) {
        return;
    }

    QString cornerId = element(svg, (m_location == Plasma::Types::LeftEdge ? "bottomright" : "topleft"));
    QImage corner = svg->image(svg->elementSize(cornerId), cornerId);

    int discovRow = (m_location == Plasma::Types::LeftEdge ? corner.height()-1 : 0);
    int discovCol{0};
    //int discovCol = (m_location == Plasma::Types::LeftEdge ? corner.width()-1 : 0);
    int round{0};

    int minOpacity = m_maxOpacity * 255;

    if (m_location == Plasma::Types::BottomEdge || m_location == Plasma::Types::RightEdge || m_location == Plasma::Types::TopEdge) {
        //! TOPLEFT corner
        //! first LEFT pixel found
        QRgba64 *line = (QRgba64 *)corner.scanLine(discovRow);

        for (int col=0; col<corner.width() - 1; ++col) {
            QRgba64 pixelData = line[col];

            if (qAlpha(pixelData) < minOpacity) {
                discovCol++;
                round++;
            } else {
                break;
            }
        }
    } else if (m_location == Plasma::Types::LeftEdge) {
        //! it should be TOPRIGHT corner in that case
        //! first RIGHT pixel found
        QRgba64 *line = (QRgba64 *)corner.scanLine(discovRow);
        for (int col=corner.width()-1; col>0; --col) {
            QRgba64 pixelData = line[col];

            if (qAlpha(pixelData) < minOpacity) {
                discovCol--;
                round++;
            } else {
                break;
            }
        }
    }

    m_roundness = round;
    emit roundnessChanged();
}

void PanelBackground::updateShadow(Plasma::Svg *svg)
{
    if (!svg) {
        return;
    }

    if (!m_parentTheme->hasShadow()) {
        m_shadowSize = 0;
        m_shadowColor = Qt::black;
        return;
    }

    bool horizontal = (m_location == Plasma::Types::BottomEdge || m_location == Plasma::Types::TopEdge);

    QString borderId{"shadow-top"};

    if  (m_location == Plasma::Types::TopEdge) {
        borderId = "shadow-bottom";
    } else if (m_location == Plasma::Types::LeftEdge) {
        borderId = "shadow-right";
    } else if (m_location == Plasma::Types::RightEdge) {
        borderId = "shadow-left";
    }

    QImage border = svg->image(svg->elementSize(borderId), borderId);

    //! find shadow size through, plasma theme
    int themeshadowsize{0};

    if  (m_location == Plasma::Types::TopEdge) {
        themeshadowsize = svg->elementSize(element(svg, "shadow-hint-bottom-margin")).height();
    } else if (m_location == Plasma::Types::LeftEdge) {
        themeshadowsize = svg->elementSize(element(svg, "shadow-hint-right-margin")).width();
    } else if (m_location == Plasma::Types::RightEdge) {
        themeshadowsize = svg->elementSize(element(svg, "shadow-hint-left-margin")).width();
    } else {
        themeshadowsize = svg->elementSize(element(svg, "shadow-hint-top-margin")).height();
    }

    //! find shadow size through heuristics, elementsize provided through svg may not be valid because it could contain
    //! many fully transparent pixels in its edges
    int discoveredshadowsize{0};
    int firstPixel{-1};
    int lastPixel{-1};

    if (horizontal) {
        for(int y = 0; y<border.height(); ++y) {
            QRgba64 *line = (QRgba64 *)border.scanLine(y);
            QRgba64 pixel = line[0];

            if (qAlpha(pixel) > 0) {
                if (firstPixel < 0) {
                    firstPixel = y;
                    lastPixel = y;
                } else {
                    lastPixel = y;
                }
            }
        }
    } else {
        QRgba64 *line = (QRgba64 *)border.scanLine(0);
        for(int x = 0; x<border.width(); ++x) {
            QRgba64 pixel = line[x];

            if (qAlpha(pixel) > 0) {
                if (firstPixel < 0) {
                    firstPixel = x;
                    lastPixel = x;
                } else {
                    lastPixel = x;
                }
            }
        }
    }

    discoveredshadowsize = (firstPixel>=0 ? qMax(0, lastPixel - firstPixel + 1) : 0);

    m_shadowSize = qMax(themeshadowsize, discoveredshadowsize);

    //! find maximum shadow color applied
    int maxopacity{0};

    for (int r=0; r<border.height(); ++r) {
        QRgba64 *line = (QRgba64 *)border.scanLine(r);

        for(int c = 0; c<border.width(); ++c) {
            QRgba64 pixel = line[c];

            if (qAlpha(pixel) > maxopacity) {
                maxopacity = qAlpha(pixel);
                m_shadowColor = QColor(pixel);
                m_shadowColor.setAlpha(qMin(255, maxopacity));
            }
        }
    }
}


void PanelBackground::updateRoundness(Plasma::Svg *svg)
{
    if (!svg) {
        return;
    }

    if (hasMask(svg)) {
        qDebug() << "PLASMA THEME, calculating roundness from mask...";
        updateRoundnessFromMask(svg);
    } else if (m_parentTheme->hasShadow()) {
        qDebug() << "PLASMA THEME, calculating roundness from shadows...";
        updateRoundnessFromShadows(svg);
    } else {
        qDebug() << "PLASMA THEME, calculating roundness from fallback code...";
        updateRoundnessFallback(svg);
    }
}

void PanelBackground::update()
{
    Plasma::Svg *backSvg = new Plasma::Svg(this);
    backSvg->setImagePath(QStringLiteral("widgets/panel-background"));
    backSvg->resize();

    updateMaxOpacity(backSvg);
    updatePaddings(backSvg);
    updateRoundness(backSvg);
    updateShadow(backSvg);

    qDebug() << " PLASMA THEME EXTENDED :: " << m_location << " | roundness:" << m_roundness << " center_max_opacity:" << m_maxOpacity;
    qDebug() << " PLASMA THEME EXTENDED :: " << m_location
             << " | padtop:" << m_paddingTop << " padleft:" << m_paddingLeft
             << " padbottom:" << m_paddingBottom << " padright:" << m_paddingRight;
    qDebug() << " PLASMA THEME EXTENDED :: " << m_location << " | shadowsize:" << m_shadowSize << " shadowcolor:" << m_shadowColor;

    backSvg->deleteLater();
}

}
}
