#!/bin/sh

usage()
{
    echo "Usage: cmake.sh [-h,--help] [-d,--debug] [-c,--clean] [--examples] [--install] [--install-prefix PREFIX]"
}

BUILD_DIR="Build/Release"
BUILD_TYPE="-DCMAKE_BUILD_TYPE=Release"
BUILD_EXAMPLES=0
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
        --examples )            BUILD_EXAMPLES=1
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

if [ $BUILD_EXAMPLES = 1 ]
then
    cmake "$BUILD_TYPE" -S "HelloWorld" -B "HelloWorld/$BUILD_DIR"
    cmake --build "HelloWorld/$BUILD_DIR" $CLEAN
else
    cmake "$BUILD_TYPE" -S "." -B "$BUILD_DIR" 
    cmake --build "$BUILD_DIR" $CLEAN
fi

if [ $INSTALL = 1 ]
then
    if [ "$INSTALL_PREFIX" = "" ]
    then
        cmake --install "$BUILD_DIR"
    else
        cmake --install "$BUILD_DIR" --prefix $"INSTALL_PREFIX"
    fi
fi
