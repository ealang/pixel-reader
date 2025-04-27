## [Pixel Reader](https://github.com/ealang/pixel-reader)

An ebook reader app for the Miyoo Mini. Supports epub and txt formats.

![Screenshot](resources/demo.gif)

## Miyoo Mini Installation

Supports Onion, MiniUI, and the default/factory OS.

1. [Download the latest release](https://github.com/ealang/pixel-reader/releases). Make sure to get the correct zip file for your OS. For Onion or default/factory OS: `pixel_reader_onion_xxx.zip`. For MiniUI: `pixel_reader_miniui_xxx.zip`. 
2. Extract the zip into the root of your SD card.
3. Boot your device, and the app should now show up in the apps/tools list.

The default location for book files is `Media/Books`.

## Development Reference

### Desktop Build

Install dependencies (Ubuntu):
```
sudo apt-get install make g++ libxml2-dev libzip-dev libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
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

Create app packages:
```
./cross-compile/miyoo-mini/create_packages.sh <version num>
```

### Key Mapping

During desktop development, the following keys are mapped to the Miyoo Mini buttons:
- Arrow keys: D-pad directions
- Space: A button
- Left Ctrl: B button
- Left Shift: X button
- Left Alt: Y button
- E: L1 button
- T: R1 button
- Tab: L2 button
- Backspace: R2 button
- Right Ctrl: Select button
- Return: Start button
- Escape: Menu button

For complete key mapping definition, see [src/sys/keymap.h](src/sys/keymap.h).

### Run Tests

[Install gtest](https://github.com/google/googletest/blob/main/googletest/README.md).

```
make test
```
