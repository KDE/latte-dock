#!/bin/bash

rm -v `find . -name '*.orig' -o -name '.directory' -o -name '*~' -o -name '*.user' -o -name '*.autosave'`
rm -rf ./build
