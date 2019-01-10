#!/bin/bash -e

# Prepare downloaded external dependencies to build

# This script must be run from core directory
ROOT=$PWD

# mkdir -p $ROOT/externals

echo "fmt: copy source"
cp -r $ROOT/downloads/fmt $ROOT/externals/fmt

WEBKIT_DIR=$ROOT/externals/WebKit
echo "WebKit: extract source"
tar -xf $ROOT/downloads/webkitgtk-2.23.2.tar.xz -C $ROOT/externals
mv $ROOT/externals/webkitgtk-2.23.2 $WEBKIT_DIR
# Patch for webkit allows to use custom version of ICU library and disables ICU
# collation
echo "WebKit: patch files"
patch -d $WEBKIT_DIR -p3 < $ROOT/scripts/WebKit.patch
echo "WebKit: copy include headers"
mkdir -p $WEBKIT_DIR/include/JavaScriptCore
cp -r $WEBKIT_DIR/Source/JavaScriptCore/API $WEBKIT_DIR/include/JavaScriptCore

SKIA_DIR=$ROOT/externals/skia
echo "Skia: copy source"
cp $ROOT/downloads/skia $SKIA_DIR -r
# Patch disables unneeded third-party deps to reduce download size
echo "Skia: patch deps"
patch -d $SKIA_DIR -p3 < $ROOT/scripts/skia.patch
echo "Skia: download third party deps"
$SKIA_DIR/tools/git-sync-deps

ICU_DIR=$ROOT/externals/icu
echo "ICU: extract source"
tar -xf $ROOT/downloads/icu4c-58_2-src.tgz -C $ROOT/externals
echo "ICU: extract data"
# unzip flag "-q" is quiet, "-o" is overwrite
unzip  -o -q $ROOT/downloads/icu4c-58_2-data.zip -d $ICU_DIR/source
echo "ICU: copy include headers"
mkdir -p $ICU_DIR/include
cp -r $ICU_DIR/source/common/unicode $ICU_DIR/include
cp -r $ICU_DIR/source/i18n/unicode $ICU_DIR/include
cp -r $ICU_DIR/source/layoutex/layout $ICU_DIR/include/layout

CATCH_DIR=$ROOT/externals/Catch2
echo "Catch2: copy source"
mkdir -p $CATCH_DIR/include/Catch2
cp $ROOT/downloads/catch.hpp $CATCH_DIR/include/Catch2/catch.hpp

echo "GLFW: extract source"
unzip -q $ROOT/downloads/glfw-3.2.1.zip -d $ROOT/externals
mv $ROOT/externals/glfw-3.2.1 $ROOT/externals/glfw
