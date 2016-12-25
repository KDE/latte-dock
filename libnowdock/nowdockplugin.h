#ifndef NOWDOCKPLUGIN_H
#define NOWDOCKPLUGIN_H

#include <QQmlExtensionPlugin>

class NowDockPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
    
public:
    void registerTypes(const char *uri);
};

#endif
