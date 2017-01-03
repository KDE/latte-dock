#!/bin/bash
#Author: Michail Vourlakos, Smith Ar
#Summary: Installation script for Latte Dock Panel
#This script was written and tested on openSuSe Leap 42.1
set -e

build_type=$1
build_type=${build_type:="Release"}

if ! [ -a build ] ; then
    mkdir build
fi

cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=$build_type ..
make
sudo make install
