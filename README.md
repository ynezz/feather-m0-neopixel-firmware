# Adafruit Feather M0 with Neopixel Wing & Sparkfun LIS331HH Firmware

This project aims to provide firmware for Adafruit Feather M0 with Neopixel Wing & Sparkfun LIS331HH Firmware.

### Current build status

* Git branch master [![CircleCI](https://circleci.com/gh/ynezz/feather-m0-neopixel-firmware/tree/master.svg?style=svg)](https://circleci.com/gh/ynezz/feather-m0-neopixel-firmware/tree/master)

## Prerequisities

* GNU Make
* CMake version >= 3.0, idealy use version 3.9.0 which @ynezz is using and we use the same on CircleCI. It should make debugging of possible errors easier. In other words, I'm not going to support build problems related to other CMake versions :-)

## Building

```shell
$ make
```

## Build tweaks

It's recommended to set any local preferences and paths via `custom.local.cmake` file.
