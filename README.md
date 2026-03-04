# CSI-ML Project

## Purpose

This example demonstrates how to use ESP-IDF to collect channel state information data from microcontroller and run
machine learning models. The device has to collect data and run models in order to predict if person in room is sitting
near desk, lying on bed or staying.

## Devices

Currently only Xiao ESP32-S3 Sense module tested.

## Collecting data

- Collecting data is done by using app that reads json data from serial port.
  It will be published in future.
- Collected data is in the format of array that consists of 112 bytes (int8_t).

## Training models

1. Tensorflow (MLP-NN, CNN)
    - Models were trained using Tensorflow, saved to `.tflite` format.
    - Then translated to C++ using `xxd` command ``` xxd -i .\model.tflite > model.cc```.
2. Alglib (Forest)
    - Model was trained and saved to `.bin` format.
3. Features - 21 statistical features from amplitude and phase, used only in NN and forest.

## Commands

### csi

| Arguments   | Alias | Options                       | Description                  |
|-------------|-------|-------------------------------|------------------------------|
| --mode      | -m    | `collect` `forest` `nn` `cnn` | You can choose mode          |
| --amount    | -a    | `<amount>`                    | Amount of packets to be sent |
| --benchmark | -b    | `none`                        | Shows memory usage           |

Usage

```bash
csi -m collect -a 20
```

## Build

- Setup your wifi connection by copying `secrets.h.template` as `secrets.h` to main folder.
- Edit `secrets.h` file and set your wifi credentials.
- Run

```bash
idf.py build
idf.py flash
idf.py monitor
```
