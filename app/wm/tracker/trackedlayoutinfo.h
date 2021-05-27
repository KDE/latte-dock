/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WINDOWSYSTEMTRACKEDLAYOUTINFO_H
#define WINDOWSYSTEMTRACKEDLAYOUTINFO_H

// local
#include "trackedgeneralinfo.h"
#include "../windowinfowrap.h"

// Qt
#include <QObject>
#include <QRect>

namespace Latte {
namespace Layout {
class GenericLayout;
}
namespace WindowSystem {
namespace Tracker {
class Windows;
}
}
}


namespace Latte {
namespace WindowSystem {
namespace Tracker {

class TrackedLayoutInfo : public TrackedGeneralInfo {
    Q_OBJECT

public:
    TrackedLayoutInfo(Tracker::Windows *tracker, Latte::Layout::GenericLayout *layout);
    ~TrackedLayoutInfo() override;

    Latte::Layout::GenericLayout *layout() const;

private:
    Latte::Layout::GenericLayout *m_layout{nullptr};
};

}
}
}

#endif
