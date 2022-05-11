/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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

    void addWindow(QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders = Plasma::FrameSvg::AllBorders);
    void removeWindow(QWindow *window);

    void setEnabledBorders(QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders = Plasma::FrameSvg::AllBorders);

private:
    class Private;
    Private *const d;
};
