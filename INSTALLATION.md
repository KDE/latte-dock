Installation
============

**Before running the installation script you have to install the dependencies needed for compiling.**

## Kubuntu 16.10

```
sudo add-apt-repository ppa:kubuntu-ppa/backports
sudo apt update 
sudo apt dist-upgrade 
sudo apt install cmake extra-cmake-modules qtdeclarative5-dev libqt5x11extras5-dev libkf5iconthemes-dev libkf5plasma-dev libkf5windowsystem-dev libkf5declarative-dev libkf5xmlgui-dev libkf5activities-dev 
```

*Now you can run the installation script*

####installation script####
- _sh install.sh_

Latte is now ready to be used by executing  ```latte-dock```