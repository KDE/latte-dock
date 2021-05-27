/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

    QString background() const;
    void setBackground(const QString &file);

    void setText(const QString &text);

    QString textColor() const;
    void setTextColor(const QString &color);

signals:
    void backgroundChanged();
    void textColorChanged();
    void mouseReleased();

protected:
    void enterEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event ) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
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
