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

#ifndef SETTINGSPATTERNWIDGET_H
#define SETTINGSPATTERNWIDGET_H

// Qt
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QWidget>


namespace Latte {
namespace Settings {
namespace Widget {

class PatternWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PatternWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void setBackground(const QString &file);
    void setText(const QString &text);
    void setTextColor(const QString &color);

signals:
    void backgroundChanged();
    void textColorChanged();

protected:
    void enterEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event ) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateUi();

private:
    void initUi();

private:
    QLabel *m_label{nullptr};

    QString m_background;
    QString m_textColor;

    float m_textColorBrightness;

    QGraphicsDropShadowEffect* m_shadowEffect{nullptr};
};

}
}
}

#endif
