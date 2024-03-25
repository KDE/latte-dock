/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QSet>

#include <KSvg/FrameSvg>
#include <KSvg/Svg>

class PanelShadows : public KSvg::Svg
{
    Q_OBJECT

public:
    explicit PanelShadows(QObject *parent = nullptr, const QString &prefix = QStringLiteral("widgets/panel-background"));
    ~PanelShadows() override;

    static PanelShadows *self();

    void addWindow(QWindow *window, KSvg::FrameSvg::EnabledBorders enabledBorders = KSvg::FrameSvg::AllBorders);
    void removeWindow(QWindow *window);

    void setEnabledBorders(QWindow *window, KSvg::FrameSvg::EnabledBorders enabledBorders = KSvg::FrameSvg::AllBorders);

private:
    class Private;
    Private *const d;
};
