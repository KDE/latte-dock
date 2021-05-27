/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "patternwidget.h"

//! local
#include "../../tools/commontools.h"

//! Qt
#include <QDebug>
#include <QFont>
#include <QHBoxLayout>
#include <QPainter>
#include <QPalette>
#include <QStyleOption>

//! KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Widget {

PatternWidget::PatternWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    setMouseTracking(true);

    setProperty("isBackground", true);

    initUi();

    connect(this, &PatternWidget::backgroundChanged, this, &PatternWidget::updateUi);
    connect(this, &PatternWidget::textColorChanged, this, &PatternWidget::updateUi);
}

void PatternWidget::initUi()
{
    QHBoxLayout *l = new QHBoxLayout(this);
    m_label = new QLabel(this);

    l->addWidget(m_label);
    l->setAlignment(Qt::AlignCenter);
    setLayout(l);

    m_label->setText(i18n("Sample Text"));
    QFont font  = m_label->font();
    font.setBold(true);
    m_label->setFont(font);

    //! shadow
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setXOffset(0);
    m_shadowEffect->setYOffset(0);
    m_label->setGraphicsEffect(m_shadowEffect);
}

QString PatternWidget::background() const
{
    return m_background;
}

void PatternWidget::setBackground(const QString &file)
{
    if (m_background != file) {
        m_background = file;
    }

    emit backgroundChanged();
}

void PatternWidget::setText(const QString &text)
{
    m_label->setText(text);
}

QString PatternWidget::textColor() const
{
    return m_textColor;
}

void PatternWidget::setTextColor(const QString &color)
{
    if (m_textColor == color) {
        return;
    }

    m_textColor = color;
    m_textColorBrightness = Latte::colorBrightness(QColor(color));

    emit textColorChanged();
}

void PatternWidget::updateUi()
{
    QPalette systemPalette;

    QColor textColor = systemPalette.color(QPalette::Active, QPalette::Text);
    QString background = "background-image: url(" + m_background + ");";

    int radius = qMax(2, height() / 4);

    if (m_background.isEmpty()) {
        background = "background-image: none;";
        m_shadowEffect->setColor(Qt::transparent);
    } else {
        m_shadowEffect->setColor("#020202");
    }

    if (m_textColorBrightness > 127) {
        m_shadowEffect->setBlurRadius(12);
    } else {
        m_shadowEffect->setBlurRadius(1);
    }

    setStyleSheet("[isBackground=true] {border: 1px solid " + textColor.name() + "; border-radius: " + QString::number(radius) + "px; " + background + "}");
    m_label->setStyleSheet("QLabel {border: 0px; background-image:none; color:" + m_textColor + "}");
}

void PatternWidget::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);
    QWidget::enterEvent(event);
}

void PatternWidget::mouseMoveEvent(QMouseEvent *event )
{
    QWidget::mouseMoveEvent(event);
}

void PatternWidget::mouseReleaseEvent(QMouseEvent *event)
{
    emit mouseReleased();
    QWidget::mouseReleaseEvent(event);
}

void PatternWidget::paintEvent(QPaintEvent *event)
{
    //! it is needed from Qt, otherwise QWidget is not updated
    //! https://wiki.qt.io/How_to_Change_the_Background_Color_of_QWidget
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

}
}
}
