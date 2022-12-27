Installation
============

## Using installation script

**Before running the installation script you have to install the dependencies needed for compiling.**


### Kubuntu only

```
sudo add-apt-repository ppa:kubuntu-ppa/backports
sudo apt update
sudo apt dist-upgrade
```

### Kubuntu and KDE Neon

```
sudo apt install cmake extra-cmake-modules qtdeclarative5-dev libqt5x11extras5-dev libkf5iconthemes-dev libkf5plasma-dev libkf5windowsystem-dev libkf5declarative-dev libkf5xmlgui-dev libkf5activities-dev build-essential libxcb-util-dev libkf5wayland-dev git gettext libkf5archive-dev libkf5notifications-dev libxcb-util0-dev libsm-dev libkf5crash-dev libkf5newstuff-dev libxcb-shape0-dev libxcb-randr0-dev libx11-dev libx11-xcb-dev kirigami2-dev libwayland-dev libwayland-client0 plasma-wayland-protocols libqt5waylandclient5-dev qtwayland5-dev-tools
```

### Arch Linux

```
sudo pacman -Syu
sudo pacman -S cmake extra-cmake-modules python plasma-framework plasma-desktop plasma-wayland-protocols
```

### Fedora/RHEL
```
sudo dnf install cmake extra-cmake-modules qt5-qtdeclarative-devel qt5-qtx11extras-devel kf5-kiconthemes-devel kf5-plasma-devel kf5-kwindowsystem-devel kf5-kdeclarative-devel kf5-kxmlgui-devel kf5-kactivities-devel gcc-c++ gcc xcb-util-devel kf5-kwayland-devel git gettext kf5-karchive-devel kf5-knotifications-devel libSM-devel kf5-kcrash-devel kf5-knewstuff-devel kf5-kdbusaddons-devel kf5-kxmlgui-devel kf5-kglobalaccel-devel kf5-kio-devel kf5-kguiaddons-devel kf5-kirigami2-devel kf5-kirigami-devel kf5-ki18n-devel qt5-qtwayland-devel plasma-wayland-protocols-devel wayland-devel
``` 

### Building and Installing

**Now you can run the installation script.**

```
sh install.sh
```

