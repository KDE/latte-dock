#ifndef WINDOWINFO_H
#define WINDOWINFO_H

#include <QObject>
#include <QWidget>
#include <QRect>

namespace Latte {

class WindowInfoWrap {

public:
    explicit WindowInfoWrap();
    WindowInfoWrap(const WindowInfoWrap &other);
    
    WindowInfoWrap &operator=(const WindowInfoWrap &rhs);
    
    bool operator==(const WindowInfoWrap &rhs) const;
    bool operator<(const WindowInfoWrap &rhs) const;
    bool operator>(const WindowInfoWrap &rhs) const;
    
    bool isValid() const;
    void setIsValid(bool isValid);
    
    bool isActive() const;
    void setIsActive(bool isActive);
    
    bool isMinimized() const;
    void setIsMinimized(bool isMinimized);
    
    bool isMaximized() const;
    void setIsMaximized(bool isMaximized);
    
    bool isFullscreen() const;
    void setIsFullscreen(bool isFullscreen);
    
    bool isOnCurrentDesktop() const;
    void setIsOnCurrentDesktop(bool isOnCurrentDesktop);
    
    QRect geometry() const;
    void setGeometry(const QRect &geometry);
    
    WId wid() const;
    void setWid(WId wid);
    
private:
    bool m_isValid : 1;
    bool m_isActive : 1;
    bool m_isMinimized : 1;
    bool m_isMaximized : 1;
    bool m_isFullscreen : 1;
    bool m_isOnCurrentDesktop : 1;
    QRect m_geometry;
    WId m_wid;
};

}

#endif // WINDOWINFO_H
