#!/bin/sh
if [ "${UNION_PLATFORM}" != "miyoomini" ]; then 
    echo "Must be run from cross-compile shell"
    exit 1
fi

cd $(dirname "$0")

make clean
make -j

STAGE_ROOT=/tmp/staging
STAGE_APP=$STAGE_ROOT/App/PixelReader

echo "Staging to $STAGE_ROOT"
rm -rf $STAGE_ROOT

mkdir -p $STAGE_APP
mkdir -p $STAGE_APP/books
mkdir -p $STAGE_APP/lib
mkdir -p $STAGE_APP/resources/fonts

cp -v resources/fonts/*.ttf $STAGE_APP/resources/fonts
cp -v resources/fonts/*.txt $STAGE_APP/resources/fonts
cp -v resources/logo/logo.png $STAGE_APP/icon.png
cp -v resources/books/*.epub $STAGE_APP/books/

cp -v resources/config.json $STAGE_APP/
cp -v resources/launch.sh   $STAGE_APP/
cp -v reader.cfg            $STAGE_APP/
cp -v README.md             $STAGE_APP

cp -v cross-compile/miyoo-mini/lib/* $STAGE_APP/lib
cp -v build/reader                   $STAGE_APP/

(cd $STAGE_ROOT && zip -r pixel_reader.zip App)

cp -v $STAGE_ROOT/pixel_reader.zip build/pixel_reader.zip
