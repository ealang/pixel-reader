# ereader

## Overview

## Build

Dependencies:
- libsdl1.2 (+image, ttf2)
- libxml2
- libzip

```
make build
```

Find app in `build/reader`.

## Miyoo Mini Cross-Compile

Cross-compile env is provided by [shauninman/union-miyoomini-toolchain](https://github.com/shauninman/union-miyoomini-toolchain). Docker is required.

Be sure to fetch submodules:

```
git submodule init && git submodule update
```

Start shell:

```
make miyoo-mini-shell
```

## Run Tests

[Install gtest](https://github.com/google/googletest/blob/main/googletest/README.md).

```
make test
```
