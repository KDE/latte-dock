#ifndef DOCK_H
#define DOCK_H

#include <QObject>
#include <QMetaEnum>
#include <QMetaType>

namespace Latte {

class Dock {
    Q_GADGET
    
public:
    Dock() = delete;
    ~Dock() {}
    
    enum Visibility {
        AlwaysVisible = 0,
        AutoHide,
        DodgeActive,
        DodgeMaximized,
        DodgeAllWindows,
    };
    Q_ENUM(Visibility)
    
    enum Alignment {
        Center = 0,
        Left,
        Right,
        Top,
        Bottom,
        Double = 10
    };
    Q_ENUM(Alignment)
    
    
    enum VisibilityState {
        /*!
         * @brief the dock is visible
         */
        Visible = 1,
        
        /*!
         * @brief the dock is hidden
         */
        Hidden = 0
    };
    Q_ENUM(VisibilityState)
};

}//end of namespace
#endif
