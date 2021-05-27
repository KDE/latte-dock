/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCHEMECOLORS_H
#define SCHEMECOLORS_H

// Qt
#include <QObject>
#include <QColor>

namespace Latte {
namespace WindowSystem {

class SchemeColors: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString schemeFile READ schemeFile NOTIFY schemeFileChanged)

    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor textColor READ textColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor inactiveBackgroundColor READ inactiveBackgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor inactiveTextColor READ inactiveTextColor NOTIFY colorsChanged)

    Q_PROPERTY(QColor highlightColor READ highlightColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor highlightedTextColor READ highlightedTextColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor positiveTextColor READ positiveTextColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor neutralTextColor READ neutralTextColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor negativeTextColor READ negativeTextColor NOTIFY colorsChanged)

    Q_PROPERTY(QColor buttonTextColor READ buttonTextColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor buttonBackgroundColor READ buttonBackgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor buttonHoverColor READ buttonHoverColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor buttonFocusColor READ buttonFocusColor NOTIFY colorsChanged)

public:
    SchemeColors(QObject *parent, QString scheme, bool plasmaTheme = false);
    ~SchemeColors() override;

    QString schemeName() const;

    QString schemeFile() const;
    void setSchemeFile(QString file);

    QColor backgroundColor() const;
    QColor textColor() const;
    QColor inactiveBackgroundColor() const;
    QColor inactiveTextColor() const;
    QColor highlightColor() const;
    QColor highlightedTextColor() const;
    QColor positiveTextColor() const;
    QColor neutralTextColor() const;
    QColor negativeTextColor() const;

    QColor buttonTextColor() const;
    QColor buttonBackgroundColor() const;
    QColor buttonHoverColor() const;
    QColor buttonFocusColor() const;

    static QString possibleSchemeFile(QString scheme);
    static QString schemeName(QString originalFile);

signals:
    void colorsChanged();
    void schemeFileChanged();

private slots:
    void updateScheme();

private:
    bool m_basedOnPlasmaTheme{false};

    QString m_schemeName;
    QString m_schemeFile;

    QColor m_activeBackgroundColor;
    QColor m_activeTextColor;

    QColor m_inactiveBackgroundColor;
    QColor m_inactiveTextColor;

    QColor m_highlightColor;
    QColor m_highlightedTextColor;
    QColor m_positiveTextColor;
    QColor m_neutralTextColor;
    QColor m_negativeTextColor;

    QColor m_buttonTextColor;
    QColor m_buttonBackgroundColor;
    QColor m_buttonHoverColor;
    QColor m_buttonFocusColor;
};

}
}

#endif
