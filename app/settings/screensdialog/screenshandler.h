/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCREENSHANDLER_H
#define SCREENSHANDLER_H

// local
#include "../generic/generichandler.h"
#include "../../data/screendata.h"

// Qt
#include <QButtonGroup>
#include <QSortFilterProxyModel>

namespace Ui {
class ScreensDialog;
}

namespace Latte{
namespace Settings{
namespace Model {
class Screens;
}
namespace Dialog{
class ScreensDialog;
}
}
}

namespace Latte {
namespace Settings {
namespace Handler {

class ScreensHandler : public Generic
{
    Q_OBJECT
public:
    ScreensHandler(Dialog::ScreensDialog *dialog);
    ~ScreensHandler();

    bool hasChangedData() const override;
    bool inDefaultValues() const override;

    Latte::Data::ScreensTable currentData() const;

public slots:
    void deselectAll();
    void reset() override;
    void resetDefaults() override;
    void save() override;

private:
    void init();
    void initDefaults();

    bool removalConfirmation(const QStringList &screens) const;

private slots:
    void onRemoveNow();
    void onScreenDataChanged();

private:
    Dialog::ScreensDialog *m_dialog{nullptr};
    Ui::ScreensDialog *m_ui{nullptr};

    //! current data
    Model::Screens *m_screensModel{nullptr};
    QSortFilterProxyModel *m_screensProxyModel{nullptr};
};

}
}
}

#endif
