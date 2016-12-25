#ifndef VISIBILITYMANAGER_H
#define VISIBILITYMANAGER_H

#include <QObject>
#include "../liblattedock/types.h"
#include "abstractinterface.h"
#include "plasmaquick/containmentview.h"

#include <QTimer>

#include <Plasma/Containment>

class VisibilityManager : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(bool disableHiding READ disableHiding WRITE setDisableHiding NOTIFY disableHidingChanged)
    //Q_PROPERTY(bool immutable READ immutable WRITE setImmutable NOTIFY immutableChanged)
    Q_PROPERTY(bool isAutoHidden READ isAutoHidden WRITE setIsAutoHidden NOTIFY isAutoHiddenChanged)
    Q_PROPERTY(bool isDockWindowType READ isDockWindowType WRITE setIsDockWindowType NOTIFY isDockWindowTypeChanged)
    Q_PROPERTY(bool isHovered READ isHovered NOTIFY isHoveredChanged)
    Q_PROPERTY(bool windowInAttention READ windowInAttention WRITE setWindowInAttention NOTIFY windowInAttentionChanged)
    
    Q_PROPERTY(LatteDock::Types::Visibility panelVisibility READ panelVisibility WRITE setPanelVisibility NOTIFY panelVisibilityChanged)
    
public:
    explicit VisibilityManager(PlasmaQuick::ContainmentView *view);
    ~VisibilityManager();
    
    bool disableHiding() const;
    void setDisableHiding(bool state);
    
    bool isAutoHidden() const;
    void setIsAutoHidden(bool state);
    
    bool isDockWindowType() const;
    void setIsDockWindowType(bool state);
    
    bool isHovered() const;
    
    bool windowInAttention() const;
    
    LatteDock::Types::Visibility panelVisibility() const;
    void setContainment(Plasma::Containment *contaiment);
    void setMaskArea(QRect area);
    void setPanelVisibility(LatteDock::Types::Visibility state);
    
public slots:
    Q_INVOKABLE void initialize();
    Q_INVOKABLE void showNormal();
    Q_INVOKABLE void showOnTop();
    Q_INVOKABLE void showOnTopCheck();
    Q_INVOKABLE void showOnBottom();
    bool event(QEvent *event);
    void setWindowInAttention(bool state);
    void updateVisibilityFlags();
    
Q_SIGNALS:
    void disableHidingChanged();
    void isAutoHiddenChanged();
    void isDockWindowTypeChanged();
    void isHoveredChanged();
    void mustBeLowered(); //are used to triger the sliding animations from the qml part
    void mustBeRaised();
    void mustBeRaisedImmediately();
    void panelVisibilityChanged();
    void windowInAttentionChanged();
    
private Q_SLOTS:
    void activeWindowChanged();
    //void compositingChanged();
    void updateState();
    void initWindow();
    void setIsHovered(bool state);
    //void screenChanged(QScreen *screen);
    //void setScreenGeometry(QRect geometry);
    //void updateWindowPosition();
    
private:
    bool m_disableHiding;
    bool m_isAutoHidden;
    bool m_isDockWindowType;
    bool m_isHovered;
    //second pass of the initialization
    bool m_secondInitPass;
    bool m_windowIsInAttention;
    
    int m_childrenLength;
    QRect m_maskArea;
    
    QTimer m_initTimer;
    QTimer m_updateStateTimer;
    
    Plasma::Containment *m_containment;
    PlasmaQuick::ContainmentView *m_view;
    
    NowDock::AbstractInterface *m_interface;
    LatteDock::Types::Visibility m_panelVisibility;
    
};


#endif

