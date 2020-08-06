#!/bin/sh

usage()
{
    echo "Usage: cmake.sh [-h,--help] [-d,--debug] [-c,--clean] [options]"
    echo ""
    echo "Options:"
    echo "  --arch <architecture>"
    echo "  --static"
    echo "  --shared"
    echo "  --framework"
    echo "  --examples"
    echo "  --install"
    echo "  --install-prefix <prefix>"
    echo ""
}

SOURCE_ROOT="."
BUILD_ROOT="Build"
BUILD_TYPE="Release"
BUILD_ARCH=""
OPTIONS=()
TARGETS=()
CLEAN=""
INSTALL=0
INSTALL_PREFIX=""

while [ "$1" != "" ]; do
    case $1 in
        -h | --help )           usage
                                exit
                                ;;
        -c | --clean )          CLEAN="--clean-first"
                                ;;
        -d | --debug )          BUILD_TYPE="Debug"
                                ;;
        --arch )                shift
                                BUILD_ARCH=$1
                                ;;
        --static )              TARGETS+=("DuckStaticLibrary")
                                ;;
        --shared )              TARGETS+=("DuckSharedLibrary")
                                ;;
        --framework )           TARGETS+=("DuckFramework")
                                ;;
        --examples )            OPTIONS+=("-DDUCK_BUILD_EXAMPLES=1")
                                TARGETS+=("HelloWorld")
                                ;;
        --install )             INSTALL=1
                                ;;
        --install-prefix )      shift
                                INSTALL_PREFIX=$1
                                INSTALL=1
                                ;;
        * )                     usage
                                exit 1
    esac
    shift
done

if [ "$BUILD_ARCH" = "" ]
then
    BUILD_DIR="$BUILD_ROOT/${BUILD_TYPE}"
else
    BUILD_DIR="$BUILD_ROOT/${BUILD_TYPE}-${BUILD_ARCH}"
fi

cmake "-DCMAKE_BUILD_TYPE=$BUILD_TYPE" ${OPTIONS[*]} -S "$SOURCE_ROOT" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --target ${TARGETS[*]} $CLEAN

if [ $INSTALL = 1 ]
then
    if [ "$INSTALL_PREFIX" = "" ]
    then
        cmake --install "$BUILD_DIR"
    else
        cmake --install "$BUILD_DIR" --prefix $"INSTALL_PREFIX"
    fi
fi
