#!/bin/bash

astyle --options='astylerc' `find -maxdepth 3 -name '*.cpp' -o -name '*.h'`
