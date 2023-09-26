#!/bin/sh

BUILD_DIR=build
INSTALL_CMD=install

if [ $# -eq 0 ]; then
    BUILD_TYPE=Release
elif [ $1 = "debug" ]; then
    BUILD_TYPE=Debug
elif [ $1 = "clean" ]; then
    INSTALL_CMD=uninstall
fi

if which ninja >/dev/null; then
    echo "Found ninja-build"
else
    sudo apt install ninja-build -y
fi

cmake -B ./${BUILD_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DENABLE_PYTHON=ON -G Ninja

#install/uninstall
cd ${BUILD_DIR} || exit
sudo ninja ${INSTALL_CMD}
cd - || exit

if [ ${INSTALL_CMD} = "uninstall" ]; then
    sudo rm -rf /usr/local/lib/ovi/plugins
fi
