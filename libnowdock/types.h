#ifndef TYPES_H
#define TYPES_H

#include <QObject>
#include <QMetaEnum>
#include <QMetaType>

namespace NowDock {

class Types {
    Q_GADGET
    
public:
    Types() = delete;
    ~Types() {}
    
    enum Visibility {
        BelowActive = 0, /** always visible except if ovelaps with the active window, no area reserved */
        BelowMaximized, /** always visible except if ovelaps with an active maximize window, no area reserved */
        LetWindowsCover, /** always visible, windows will go over the panel, no area reserved */
        WindowsGoBelow, /** default, always visible, windows will go under the panel, no area reserved */
        AutoHide, /** the panel will be shownn only if the mouse cursor is on screen edges */
        AlwaysVisible,  /** always visible panel, "Normal" plasma panel, accompanies plasma's "Always Visible"  */
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
