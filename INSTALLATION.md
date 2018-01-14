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
sudo apt install cmake extra-cmake-modules qtdeclarative5-dev libqt5x11extras5-dev libkf5iconthemes-dev libkf5plasma-dev libkf5windowsystem-dev libkf5declarative-dev libkf5xmlgui-dev libkf5activities-dev build-essential libxcb-util-dev libkf5wayland-dev git gettext libkf5archive-dev libkf5notifications-dev libxcb-util0-dev libsm-dev libkf5crash-dev libkf5newstuff-dev
```

### Arch Linux

```
sudo pacman -Syu
sudo pacman -S cmake extra-cmake-modules python plasma-framework plasma-desktop
```

### Building and Installing

**Now you can run the installation script.**

```
sh install.sh
```

