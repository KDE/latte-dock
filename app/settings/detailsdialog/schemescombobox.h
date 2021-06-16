/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCHEMESCOMBOBOX_H
#define SCHEMESCOMBOBOX_H

//Qt
#include <QComboBox>
#include <QColor>
#include <QPaintEvent>

namespace Latte {
namespace Settings {

class SchemesComboBox : public QComboBox
{
  Q_OBJECT
public:
  SchemesComboBox(QWidget *parent = nullptr);

  QColor backgroundColor() const;
  void setBackgroundColor(const QColor &color);

  QColor textColor() const;
  void setTextColor(const QColor &color);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QColor m_backgroundColor;
  QColor m_textColor;

};

}
}

#endif
