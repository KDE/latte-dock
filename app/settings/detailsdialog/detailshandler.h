/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DETAILSDIALOGHANDLER_H
#define DETAILSDIALOGHANDLER_H

// local
#include "../generic/generichandler.h"
#include "../../data/layoutdata.h"
#include "../../layout/abstractlayout.h"

// Qt
#include <QButtonGroup>
#include <QSortFilterProxyModel>

namespace Ui {
class DetailsDialog;
}

namespace Latte{
namespace Settings{
namespace Dialog{
class DetailsDialog;
}
}
}

namespace Latte{
namespace Settings{
namespace Model {
class Colors;
}
}
}


namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class DetailsHandler : public Generic
{
    Q_OBJECT
public:
    DetailsHandler(Dialog::DetailsDialog *parentDialog);
    ~DetailsHandler();

    bool hasChangedData() const override;
    bool inDefaultValues() const override;

    Latte::Data::Layout currentData() const;

public slots:
    void reset() override;
    void resetDefaults() override;
    void save() override;

signals:
    void currentLayoutChanged();

private slots:
    void onCurrentLayoutIndexChanged(int row);
    void onCurrentColorIndexChanged(int row);

    void clearIcon();
    void clearPattern();
    void selectBackground();
    void selectIcon();
    void selectTextColor();
    void updateWindowTitle();

private:
    void init();
    void reload();

    void setIsShownInMenu(bool inMenu);
    void setHasDisabledBorders(bool disabled);

    void setBackground(const QString &background);
    void setTextColor(const QString &textColor);
    void setColor(const QString &color);
    void setIcon(const QString &icon);

    void setBackgroundStyle(const Latte::Layout::BackgroundStyle &style);

    void loadLayout(const Latte::Data::Layout &data);

    int saveChanges();

private:
    Dialog::DetailsDialog *m_parentDialog{nullptr};
    Ui::DetailsDialog *m_ui{nullptr};

    QSortFilterProxyModel *m_layoutsProxyModel{nullptr};

    //! current data
    Model::Colors *m_colorsModel{nullptr};

    QButtonGroup *m_backButtonsGroup;

    Latte::Data::Layout o_data;
    Latte::Data::Layout c_data;
};

}
}
}

#endif
