/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "panelshadows_p.h"

#include <QDebug>
#include <KWindowShadow>

class PanelShadows::Private
{
public:
    Private(PanelShadows *shadows)
        : q(shadows)
    {
    }

    ~Private()
    {
    }

    void clearTiles();
    void setupTiles();
    void initTile(const QString &element);
    void updateShadow(QWindow *window, KSvg::FrameSvg::EnabledBorders);
    void clearShadow(QWindow *window);
    void updateShadows();
    bool hasShadows() const;

    PanelShadows *q;

    QHash<QWindow *, KSvg::FrameSvg::EnabledBorders> m_windows;
    QHash<QWindow *, KWindowShadow *> m_shadows;
    QVector<KWindowShadowTile::Ptr> m_tiles;
};

class PanelShadowsSingleton
{
public:
    PanelShadowsSingleton()
    {
    }

    PanelShadows self;
};

Q_GLOBAL_STATIC(PanelShadowsSingleton, privatePanelShadowsSelf)

PanelShadows::PanelShadows(QObject *parent, const QString &prefix)
    : KSvg::Svg(parent)
    , d(new Private(this))
{
    setImagePath(prefix);
    connect(this, &KSvg::Svg::repaintNeeded, this, [this]() {
        d->updateShadows();
    });
}

PanelShadows::~PanelShadows()
{
    delete d;
}

PanelShadows *PanelShadows::self()
{
    return &privatePanelShadowsSelf->self;
}

void PanelShadows::addWindow(QWindow *window, KSvg::FrameSvg::EnabledBorders enabledBorders)
{
    if (!window) {
        return;
    }

    d->m_windows[window] = enabledBorders;
    d->updateShadow(window, enabledBorders);
    connect(window, &QObject::destroyed, this, [this, window]() {
        d->m_windows.remove(window);
        d->clearShadow(window);
        if (d->m_windows.isEmpty()) {
            d->clearTiles();
        }
    });
}

void PanelShadows::removeWindow(QWindow *window)
{
    if (!d->m_windows.contains(window)) {
        return;
    }

    d->m_windows.remove(window);
    disconnect(window, nullptr, this, nullptr);
    d->clearShadow(window);

    if (d->m_windows.isEmpty()) {
        d->clearTiles();
    }
}

void PanelShadows::setEnabledBorders(QWindow *window, KSvg::FrameSvg::EnabledBorders enabledBorders)
{
    if (!window || !d->m_windows.contains(window)) {
        return;
    }

    d->m_windows[window] = enabledBorders;
    d->updateShadow(window, enabledBorders);
}

void PanelShadows::Private::updateShadows()
{
    const bool hadShadowsBefore = !m_tiles.isEmpty();

    // has shadows now?
    if (hasShadows()) {
        if (hadShadowsBefore) {
            clearTiles();
        }
        for (auto i = m_windows.constBegin(); i != m_windows.constEnd(); ++i) {
            updateShadow(i.key(), i.value());
        }
    } else {
        if (hadShadowsBefore) {
            for (auto i = m_windows.constBegin(); i != m_windows.constEnd(); ++i) {
                clearShadow(i.key());
            }
            clearTiles();
        }
    }
}

void PanelShadows::Private::initTile(const QString &element)
{
    const QImage image = q->pixmap(element).toImage();

    KWindowShadowTile::Ptr tile = KWindowShadowTile::Ptr::create();
    tile->setImage(image);

    m_tiles << tile;
}

void PanelShadows::Private::setupTiles()
{
    clearTiles();

    initTile(QStringLiteral("shadow-top"));
    initTile(QStringLiteral("shadow-topright"));
    initTile(QStringLiteral("shadow-right"));
    initTile(QStringLiteral("shadow-bottomright"));
    initTile(QStringLiteral("shadow-bottom"));
    initTile(QStringLiteral("shadow-bottomleft"));
    initTile(QStringLiteral("shadow-left"));
    initTile(QStringLiteral("shadow-topleft"));
}

void PanelShadows::Private::clearTiles()
{
    m_tiles.clear();
}

void PanelShadows::Private::updateShadow(QWindow *window, KSvg::FrameSvg::EnabledBorders enabledBorders)
{
    if (!hasShadows()) {
        return;
    }

    if (m_tiles.isEmpty()) {
        setupTiles();
    }

    KWindowShadow *&shadow = m_shadows[window];

    if (!shadow) {
        shadow = new KWindowShadow(q);
    }

    if (shadow->isCreated()) {
        shadow->destroy();
    }

    if (enabledBorders & KSvg::FrameSvg::TopBorder) {
        shadow->setTopTile(m_tiles.at(0));
    } else {
        shadow->setTopTile(nullptr);
    }

    if (enabledBorders & KSvg::FrameSvg::TopBorder && enabledBorders & KSvg::FrameSvg::RightBorder) {
        shadow->setTopRightTile(m_tiles.at(1));
    } else {
        shadow->setTopRightTile(nullptr);
    }

    if (enabledBorders & KSvg::FrameSvg::RightBorder) {
        shadow->setRightTile(m_tiles.at(2));
    } else {
        shadow->setRightTile(nullptr);
    }

    if (enabledBorders & KSvg::FrameSvg::BottomBorder && enabledBorders & KSvg::FrameSvg::RightBorder) {
        shadow->setBottomRightTile(m_tiles.at(3));
    } else {
        shadow->setBottomRightTile(nullptr);
    }

    if (enabledBorders & KSvg::FrameSvg::BottomBorder) {
        shadow->setBottomTile(m_tiles.at(4));
    } else {
        shadow->setBottomTile(nullptr);
    }

    if (enabledBorders & KSvg::FrameSvg::BottomBorder && enabledBorders & KSvg::FrameSvg::LeftBorder) {
        shadow->setBottomLeftTile(m_tiles.at(5));
    } else {
        shadow->setBottomLeftTile(nullptr);
    }

    if (enabledBorders & KSvg::FrameSvg::LeftBorder) {
        shadow->setLeftTile(m_tiles.at(6));
    } else {
        shadow->setLeftTile(nullptr);
    }

    if (enabledBorders & KSvg::FrameSvg::TopBorder && enabledBorders & KSvg::FrameSvg::LeftBorder) {
        shadow->setTopLeftTile(m_tiles.at(7));
    } else {
        shadow->setTopLeftTile(nullptr);
    }

    QMargins padding;

    if (enabledBorders & KSvg::FrameSvg::TopBorder) {
        const QSize marginHint = q->elementSize(QStringLiteral("shadow-hint-top-margin")).toSize();
        if (marginHint.isValid()) {
            padding.setTop(marginHint.height());
        } else {
            padding.setTop(m_tiles[0]->image().height());
        }
    }

    if (enabledBorders & KSvg::FrameSvg::RightBorder) {
        const QSize marginHint = q->elementSize(QStringLiteral("shadow-hint-right-margin")).toSize();
        if (marginHint.isValid()) {
            padding.setRight(marginHint.width());
        } else {
            padding.setRight(m_tiles[2]->image().width());
        }
    }

    if (enabledBorders & KSvg::FrameSvg::BottomBorder) {
        const QSize marginHint = q->elementSize(QStringLiteral("shadow-hint-bottom-margin")).toSize();
        if (marginHint.isValid()) {
            padding.setBottom(marginHint.height());
        } else {
            padding.setBottom(m_tiles[4]->image().height());
        }
    }

    if (enabledBorders & KSvg::FrameSvg::LeftBorder) {
        const QSize marginHint = q->elementSize(QStringLiteral("shadow-hint-left-margin")).toSize();
        if (marginHint.isValid()) {
            padding.setLeft(marginHint.width());
        } else {
            padding.setLeft(m_tiles[6]->image().width());
        }
    }

    shadow->setPadding(padding);
    shadow->setWindow(window);

    if (!shadow->create()) {
        qDebug() << "Couldn't create KWindowShadow for" << window;
    }
}

void PanelShadows::Private::clearShadow(QWindow *window)
{
    delete m_shadows.take(window);
}

bool PanelShadows::Private::hasShadows() const
{
    return q->hasElement(QStringLiteral("shadow-left"));
}

#include "moc_panelshadows_p.cpp"
