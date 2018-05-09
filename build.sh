#!/bin/bash
MACHINE="$(gcc -dumpmachine)"
# echo Building for ${MACHINE}...
mkdir -p "build_${MACHINE}"
cd "build_${MACHINE}"
rm -f *.pak
cmake -GNinja ..
ninja
cd ..