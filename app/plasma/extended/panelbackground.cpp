/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "panelbackground.h"

// Qt
#include <QImage>

#define CENTERWIDTH 100
#define CENTERHEIGHT 50


namespace Latte {
namespace PlasmaExtended {

PanelBackground::PanelBackground(Plasma::Types::Location edge, QObject *parent)
    : QObject(parent),
      m_location(edge)
{
}

PanelBackground::~PanelBackground()
{
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

float PanelBackground::maxOpacity() const
{
    return m_maxOpacity;
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
        QRgb *line = (QRgb *)center.scanLine(row);

        for (int col=0; col<CENTERWIDTH; ++col) {
            QRgb pixelData = line[col];
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

void PanelBackground::updateRoundness(Plasma::Svg *svg)
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
        QRgb *line = (QRgb *)corner.scanLine(discovRow);

        for (int col=0; col<corner.width() - 1; ++col) {
            QRgb pixelData = line[col];

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
        QRgb *line = (QRgb *)corner.scanLine(discovRow);
        for (int col=corner.width()-1; col>0; --col) {
            QRgb pixelData = line[col];

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

void PanelBackground::update()
{
    Plasma::Svg *backSvg = new Plasma::Svg(this);
    backSvg->setImagePath(QStringLiteral("widgets/panel-background"));
    backSvg->resize();

    updateMaxOpacity(backSvg);
    updatePaddings(backSvg);
    updateRoundness(backSvg);

    qDebug() << " PLASMA THEME EXTENDED :: " << m_location << " | roundness:" << m_roundness << " center_max_opacity:" << m_maxOpacity;
    qDebug() << " PLASMA THEME EXTENDED :: " << m_location
             << " | padtop:" << m_paddingTop << " padleft:" << m_paddingLeft
             << " padbottom:" << m_paddingBottom << " padright:" << m_paddingRight;

    backSvg->deleteLater();
}

}
}
