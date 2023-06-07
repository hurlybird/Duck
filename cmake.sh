#!/bin/sh

usage()
{
    echo "Usage: cmake.sh [-h,--help] [-d,--debug] [-r,--release] [-c,--clean] [options]"
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
BUILD_ROOT="_Build"
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
        -r | --release )        BUILD_TYPE="Release"
                                ;;
        --arch )                shift
                                BUILD_ARCH=$1
                                ;;
        --static )              TARGETS+=("STATIC")
                                ;;
        --shared )              TARGETS+=("SHARED")
                                ;;
        --framework )           TARGETS+=("FRAMEWORK")
                                ;;
        --examples )            TARGETS+=("EXAMPLES")
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

if [ ${#TARGETS[@]} -eq 0 ]
then
    TARGETS="STATIC;SHARED;FRAMEWORK;EXAMPLES"
else
    printf -v TARGETS '%s;' "${TARGETS[@]}";
fi

cmake "-DCMAKE_BUILD_TYPE=$BUILD_TYPE" "-DDUCK_BUILD=${TARGETS}" ${OPTIONS[*]} -S "$SOURCE_ROOT" -B "$BUILD_DIR"
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
