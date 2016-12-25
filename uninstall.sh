#!/bin/bash
#Author: Michail Vourlakos
#Summary: Uninstallation script for Now Dock Panel
#This script was written and tested on openSuSe Leap 42.1

if [ -f build/install_manifest.txt ]; then
   echo "Uninstallation file exists..."
   sudo xargs rm < build/install_manifest.txt
else
   echo "Uninstallation file does not exist."
fi
