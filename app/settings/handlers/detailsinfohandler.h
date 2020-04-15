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

#ifndef DETAILSINFOHANDLER_H
#define DETAILSINFOHANDLER_H

//! local
#include "generichandler.h"
#include "../data/layoutdata.h"


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
namespace Handler {
class DetailsHandler;
}
}
}


namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class DetailsInfoHandler : public Generic
{
    Q_OBJECT
public:
    DetailsInfoHandler(Dialog::DetailsDialog *parentDialog, DetailsHandler *parentHandler);
    ~DetailsInfoHandler();

    bool dataAreChanged() const override;
    bool inDefaultValues() const override;

    void reset() override;
    void resetDefaults() override;
    void save() override;

private slots:
    void init();
    void reload();

private:
    void loadLayout(const Data::Layout &data);

private:
    Dialog::DetailsDialog *m_parentDialog{nullptr};
    Ui::DetailsDialog *m_ui{nullptr};

    DetailsHandler *m_parentHandler{nullptr};

    QButtonGroup *m_backButtonsGroup;
};

}
}
}

#endif
