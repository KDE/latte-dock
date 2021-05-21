/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PANELSHADOWS_P_H
#define PANELSHADOWS_P_H

#include <QSet>

#include "plasma/framesvg.h"
#include "plasma/svg.h"


class PanelShadows : public Plasma::Svg
{
    Q_OBJECT

public:
    explicit PanelShadows(QObject *parent = nullptr, const QString &prefix = QStringLiteral("widgets/panel-background"));
    ~PanelShadows() override;

    static PanelShadows *self();

    void addWindow(const QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders = Plasma::FrameSvg::AllBorders);
    void removeWindow(const QWindow *window);

    void setEnabledBorders(const QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders = Plasma::FrameSvg::AllBorders);

    bool hasShadows() const;

private:
    class Private;
    Private * const d;
};

#endif

