#!/bin/sh

usage()
{
    echo "Usage: cmake.sh [-h,--help] [-d,--debug] [-c,--clean] [--examples] [--install] [--install-prefix PREFIX]"
}

SOURCE_DIR="."
BUILD_DIR="Build/Release"
BUILD_TYPE="-DCMAKE_BUILD_TYPE=Release"
CLEAN=""
INSTALL=0
INSTALL_PREFIX=""

while [ "$1" != "" ]; do
    case $1 in
        -c | --clean )          CLEAN="--clean-first"
                                ;;
        -d | --debug )          BUILD_TYPE="-DCMAKE_BUILD_TYPE=Debug"
                                BUILD_DIR="Build/Debug"
                                ;;
        --examples )            SOURCE_DIR="HelloWorld"
                                ;;
        -h | --help )           usage
                                exit
                                ;;
        --install )             INSTALL=1
                                ;;
        --install-prefix )      shift
                                INSTALL_PREFIX=$1
                                ;;
        * )                     usage
                                exit 1
    esac
    shift
done

mkdir -p $BUILD_DIR

cmake "$BUILD_TYPE" -S "$SOURCE_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" $CLEAN

if [ $INSTALL = 1 ]
then
    if [ "$INSTALL_PREFIX" = "" ]
    then
        cmake --install "$BUILD_DIR"
    else
        cmake --install "$BUILD_DIR" --prefix $"INSTALL_PREFIX"
    fi
fi
