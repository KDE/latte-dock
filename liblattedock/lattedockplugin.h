#ifndef LATTEDOCKPLUGIN_H
#define LATTEDOCKPLUGIN_H

#include <QQmlExtensionPlugin>

class LatteDockPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
    
public:
    void registerTypes(const char *uri);
};

#endif
