/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTLAYOUT_H
#define ABSTRACTLAYOUT_H

// Qt
#include <QObject>

// KDE
#include <KConfigGroup>

// Plasma
#include <Plasma>

namespace Plasma {
class Types;
}

namespace Latte {
namespace Layout {
Q_NAMESPACE

enum Type {
    Abstract = 0,
    Generic,
    Central,
    Shared
};
Q_ENUM_NS(Type);

enum BackgroundStyle
{
    ColorBackgroundStyle = 0,
    PatternBackgroundStyle
};
Q_ENUM_NS(BackgroundStyle);

}
}

namespace Latte {
namespace Layout {

const char MULTIPLELAYOUTSHIDDENNAME[] = ".multiple-layouts_hidden";

class AbstractLayout : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)

    Q_PROPERTY(bool preferredForShortcutsTouched READ preferredForShortcutsTouched WRITE setPreferredForShortcutsTouched NOTIFY preferredForShortcutsTouchedChanged)

    Q_PROPERTY(int popUpMargin READ popUpMargin WRITE setPopUpMargin NOTIFY popUpMarginChanged)

    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString background READ background NOTIFY backgroundChanged)
    Q_PROPERTY(QString textColor READ textColor NOTIFY textColorChanged)

    Q_PROPERTY(QString schemeFile READ schemeFile WRITE setSchemeFile NOTIFY schemeFileChanged)

    Q_PROPERTY(QStringList launchers READ launchers WRITE setLaunchers NOTIFY launchersChanged)
    Q_PROPERTY(QString lastUsedActivity READ lastUsedActivity NOTIFY lastUsedActivityChanged)

public:
    AbstractLayout(QObject *parent, QString layoutFile, QString assignedName = QString());
    ~AbstractLayout() override;

    int version() const;
    void setVersion(int ver);

    bool preferredForShortcutsTouched() const;
    void setPreferredForShortcutsTouched(bool touched);

    int popUpMargin() const;
    void setPopUpMargin(const int &margin);

    QString lastUsedActivity() const;
    void clearLastUsedActivity(); //!e.g. when we export a layout

    QString name() const;
    QString file() const;

    virtual QString background() const;

    QString color() const;
    void setColor(QString color);

    QString customBackground() const;
    void setCustomBackground(const QString &background);

    QString customTextColor() const;
    void setCustomTextColor(const QString &customColor);

    QString icon() const;
    void setIcon(const QString &icon);

    QString predefinedTextColor() const;

    QString schemeFile() const;
    void setSchemeFile(const QString &file);

    virtual QString textColor() const;
    void setTextColor(QString color);

    BackgroundStyle backgroundStyle() const;
    void setBackgroundStyle(const BackgroundStyle &style);

    QStringList launchers() const;
    void setLaunchers(QStringList launcherList);

    virtual Type type() const;

    void syncSettings();

// STATIC
    static QString defaultCustomTextColor();
    static QString defaultCustomBackground();
    static QString defaultTextColor(const QString &color);
    static QString layoutName(const QString &fileName);
    static QList<Plasma::Types::Location> combinedFreeEdges(const QList<Plasma::Types::Location> &edges1,
                                                            const QList<Plasma::Types::Location> &edges2);

signals:
    void backgroundChanged();
    void backgroundStyleChanged();
    void customBackgroundChanged();
    void customTextColorChanged();
    void colorChanged();
    void fileChanged();
    void iconChanged();
    void lastUsedActivityChanged();
    void launchersChanged();
    void nameChanged();
    void popUpMarginChanged();
    void preferredForShortcutsTouchedChanged();
    void schemeFileChanged();
    void textColorChanged();
    void versionChanged();

protected slots:
    void loadConfig();
    void saveConfig();

protected:
    void init();
    void setName(QString name);
    void setFile(QString file);

protected:
    bool m_loadedCorrectly{false};
    bool m_preferredForShortcutsTouched{false};

    //if version doesn't exist it is and old layout file
    int m_version{2};

    int m_popUpMargin{-1}; //default

    QString m_customBackground;
    QString m_customTextColor;
    QString m_color;
    QString m_lastUsedActivity; //the last used activity for this layout
    QString m_icon;
    QString m_schemeFile;

    BackgroundStyle m_backgroundStyle{ColorBackgroundStyle};

    QString m_layoutFile;
    QString m_layoutName;

    QStringList m_launchers;

    KConfigGroup m_layoutGroup;

};

}
}

#endif
