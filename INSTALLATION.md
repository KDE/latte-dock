Installation
============

## 1. Dependencies

**Before running the installation script you have to install the dependencies needed for compiling.**


### Debian 9.0 (Stretch)

```
sudo apt install g++ cmake extra-cmake-modules qtdeclarative5-dev libqt5x11extras5-dev libkf5iconthemes-dev libkf5plasma-dev libkf5windowsystem-dev libkf5declarative-dev libkf5xmlgui-dev libkf5activities-dev gettext libkf5wayland-dev libxcb-util0-dev
```

### Kubuntu only

```
sudo add-apt-repository ppa:kubuntu-ppa/backports
sudo apt update 
sudo apt dist-upgrade 
```

### Kubuntu and KDE Neon

```
sudo apt install cmake extra-cmake-modules qtdeclarative5-dev libqt5x11extras5-dev libkf5iconthemes-dev libkf5plasma-dev libkf5windowsystem-dev libkf5declarative-dev libkf5xmlgui-dev libkf5activities-dev build-essential libxcb-util-dev libkf5wayland-dev git gettext libkf5archive-dev libkf5notifications-dev libxcb-util0-dev
```

### Arch Linux

```
sudo pacman -Syy
sudo pacman -S cmake extra-cmake-modules
sudo pacman -S qt5-base qt5-declarative qt5-x11extras
sudo pacman -S kiconthemes kdbusaddons kxmlgui kdeclarative plasma-framework plasma-desktop
```

## 2. Building and Installing

**Now you can run the installation script.**

```
sh install.sh
```

Latte is now ready to be used by executing  ```latte-dock``` or _Latte Dock_ in applications menu.
