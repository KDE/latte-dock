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

#include "patternwidget.h"

//! Qt
#include <QDebug>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>

//! KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Widget {

PatternWidget::PatternWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
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
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setColor(Qt::black);
    effect->setBlurRadius(3);
    effect->setXOffset(0);
    effect->setYOffset(0);
    m_label->setGraphicsEffect(effect);
}

void PatternWidget::setBackground(const QString &file)
{
    if (m_background == file) {
        return;
    }

    m_background = file;
    emit backgroundChanged();
}

void PatternWidget::setText(const QString &text)
{
    m_label->setText(text);
}


void PatternWidget::setTextColor(const QString &color)
{
    if (m_textColor == color) {
        return;
    }

    m_textColor = color;
    emit textColorChanged();
}

void PatternWidget::updateUi()
{
    QString backgroundImage = "url(" + m_background + ");";

    if (m_background.isEmpty()) {
        backgroundImage = "none;";
    }

    setStyleSheet("[isBackground=true] {border: 1px solid black; border-radius: 8px; background-image: " + backgroundImage + "}");
    m_label->setStyleSheet("QLabel {border: 0px; background-image:none; color:" + m_textColor + "}");
}

void PatternWidget::paintEvent(QPaintEvent* event)
{
    //! it is needed from Qt, otherwise QWidget is not updated
    //! https://wiki.qt.io/How_to_Change_the_Background_Color_of_QWidget
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

}
}
}
