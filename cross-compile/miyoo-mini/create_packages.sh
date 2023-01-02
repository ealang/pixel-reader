#!/bin/sh
set -e
if [ "${UNION_PLATFORM}" != "miyoomini" ]; then
    echo "Must be run from cross-compile shell"
    exit 1
fi

cd $(dirname "$0")/../..

VERSION=$1
if [ -z "$VERSION" ]; then
    echo "Version arg is required, e.g. '1.0'"
    exit 1
fi
echo "Version v${VERSION}"

make clean
make -j

stage_common() {
    local STAGE_ROOT=$1

    mkdir -p $STAGE_APP
    mkdir -p $STAGE_APP/lib
    mkdir -p $STAGE_APP/resources/fonts

    cp -v resources/fonts/*.ttf $STAGE_APP/resources/fonts
    cp -v resources/fonts/*.txt $STAGE_APP/resources/fonts

    cp -v README.md           $STAGE_APP
    cp -v cross-compile/miyoo-mini/launch.sh $STAGE_APP/

    local LIB_SRC=cross-compile/miyoo-mini/lib
    cp -v $LIB_SRC/libSDL_image-1.2.so.0 $STAGE_APP/lib
    cp -v $LIB_SRC/libjpeg.so.8 $STAGE_APP/lib
    cp -v $LIB_SRC/liblzma.so.5 $STAGE_APP/lib
    cp -v $LIB_SRC/libxml2.so.2 $STAGE_APP/lib
    cp -v $LIB_SRC/libz.so.1    $STAGE_APP/lib
    cp -v $LIB_SRC/libzip.so.5  $STAGE_APP/lib

    cp -v build/reader $STAGE_APP/
}

create_onion_pkg() {
    local STAGE_ROOT=/tmp/staging_onion
    local STAGE_APP=$STAGE_ROOT/App/PixelReader

    echo "------------------------------------------"
    echo "Onion: Staging to $STAGE_ROOT"
    rm -rf $STAGE_ROOT

    stage_common $STAGE_ROOT

    cp -v resources/icon/icon.png $STAGE_APP/icon.png
    cat cross-compile/miyoo-mini/config.json | sed "s/VERSION/${VERSION}/" | tee $STAGE_APP/config.json

    local FILENAME="pixel_reader_onion_v${VERSION}.zip"
    (cd $STAGE_ROOT && zip -r $FILENAME App)
    cp -v $STAGE_ROOT/$FILENAME build/
}

create_miniui_pkg() {
    local STAGE_ROOT=/tmp/staging_miniui
    local STAGE_APP=$STAGE_ROOT/Tools/PixelReader.pak

    echo "------------------------------------------"
    echo "MiniUI: Staging to $STAGE_ROOT"
    rm -rf $STAGE_ROOT

    stage_common $STAGE_ROOT

    echo $VERSION > $STAGE_APP/version

    local FILENAME="pixel_reader_miniui_v${VERSION}.zip"
    (cd $STAGE_ROOT && zip -r $FILENAME Tools)
    cp -v $STAGE_ROOT/$FILENAME build/
}

create_onion_pkg
create_miniui_pkg
