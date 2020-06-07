#!/bin/bash

set -o verbose

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get -qq update
sudo apt-get -y install gcc-8 g++-8 gcc-8-multilib g++-8-multilib
sudo apt-get install libsdl2-dev:i386 libjpeg8-dev:i386 libcurl4-openssl-dev:i386
sudo apt-get install p7zip-full

cmake --version

# Build ETe

#export BUILD_CONFIGURATION="Debug"
export BUILD_CONFIGURATION="Release"

cd src
mkdir build
cd build

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="$BUILD_CONFIGURATION" -DUSE_SDL2=TRUE -DCMAKE_TOOLCHAIN_FILE="../CMakeModules/linux-i686.cmake" || exit 1
cmake --build . --config $BUILD_CONFIGURATION || exit 1

ls -R *.x86

echo "$TRAVIS_TAG"
echo "$BUILD_CONFIGURATION"

7z a "$TRAVIS_BUILD_DIR/ete-linux-$TRAVIS_TAG-$BUILD_CONFIGURATION.x86.7z" ete.x86 eteded.x86
cd "$TRAVIS_BUILD_DIR"
7z a "ete-linux-$TRAVIS_TAG-$BUILD_CONFIGURATION.x86.7z" docs\*
