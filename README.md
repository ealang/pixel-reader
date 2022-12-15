## Pixel Reader

An epub reader for Linux. It is currently optimized for the [Miyoo Mini](https://retrogamecorps.com/2022/05/15/miyoo-mini-v2-guide/).

The app is in an early release state. Images are not supported. Text only, and limited formatting.

![Screenshot](resources/demo.gif)

## Miyoo Mini Installation

Instructions for Onion or default OS:

[Download the latest release](https://github.com/ealang/pixel-reader/releases), and extract the zip into the root of your SD card. Boot your device, and the app should now show up in the apps list.

Epub files can be placed anywhere on the SD card. Browse to the location after launching the app.

## Development Reference

### Desktop Build

Install dependencies (Ubuntu):
```
apt install make c++ libxml2-dev libzip-dev libsdl1.2-dev libsdl-ttf2.0-dev
```

Build:
```
make -j
```

Find app in `build/reader`.

### Miyoo Mini Cross-Compile

Cross-compile env is provided by [shauninman/union-miyoomini-toolchain](https://github.com/shauninman/union-miyoomini-toolchain). Docker is required.

Fetch git submodules:
```
git submodule init && git submodule update
```

Start shell:
```
make miyoo-mini-shell
```

Create app package:
```
./miyoo_mini_package.sh <version num>
```

### Run Tests

[Install gtest](https://github.com/google/googletest/blob/main/googletest/README.md).

```
make test
```
