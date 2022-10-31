# Bluenet microapp

A microapp is like an addon for a Crownstone.
The goal of microapps is to make it easy to run additional code on top of released bluenet firmware.

Be careful what microapp you upload though. While there are built in safety measures, there are always ways to make your crownstone unaccassible with a microapp. It's best to first test a microapp on a development kit.

# Organization

The files that form the backbone of microapps can be found in `include` and `src`.
There are a lot of examples in `examples`.
The results of a build can be found in the `build` directory.
More details can be found in the `docs` dir.

# Configuration

By default, the microapp builds for the nRF52832 chip. This can be changed in the Makefile, by setting `TARGET_CONFIG_FILE`.

Create a `private.mk` file to override some default configs.

In order to build a microapp, you need to specify where to find the bluenet repository:
```
# The path to the bluenet repository
BLUENET_PATH=$(HOME)/workspace/bluenet

# Path to the gcc arm none eabi compiler (do not use another one).
GCC_PATH=$(BLUENET_PATH)/tools/gcc_arm_none_eabi/bin
```

The example to build can be set by changing `TARGET_NAME`:
```
TARGET_NAME=hello
```

For uploading via BLE or UART, you will need a link to a file somewhere private (say `~/.crownstone/keys`) which contains your keys.
For BLE, you will also need the address of the Crownstone.
For UART, the device should be set, and if you want to see the bluenet debug logs, also set the file with extracted log strings.
```
KEYS_JSON=~/.crownstone/keys/your_sphere_keys.json
BLE_ADDRESS=XX:XX:XX:XX:XX:XX
UART_DEVICE=/dev/ttyACM0
LOG_STRINGS_FILE=$(BLUENET_PATH)/build/default/extracted_logs.json
```

The file `your_sphere_keys.json` has the format as stipulated in the [python lib](https://github.com/crownstone/crownstone-lib-python-ble) documentation:
```
}
  "admin": "ffffffffffffffffffffffffffffffff",
  "member": "ffffffffffffffffffffffffffffffff",
  "guest": "ffffffffffffffffffffffffffffffff",
  "basic": "ffffffffffffffffffffffffffffffff",
  "serviceDataKey": "ffffffffffffffffffffffffffffffff",
  "localizationKey": "ffffffffffffffffffffffffffffffff",
  "meshApplicationKey": "ffffffffffffffffffffffffffffffff",
  "meshNetworkKey": "ffffffffffffffffffffffffffffffff"
}
```

Most likely only the admin key is required (but the lib might complain if others are missing).

# Building

To build the microapp use:
```
make clean
make
```

# Printing

Release firmware has no debug logs. This includes prints from the microapps.
If you want `println()` to work, you will have to rebuild bluenet with `CS_SERIAL_ENABLED=SERIAL_ENABLE_RX_AND_TX` and `SERIAL_VERBOSITY=SERIAL_INFO`.

Another option would be to use the `Message` class. This class also works for release firmware, but you will have to write your own client (using the crownstone uart library).

# Uploading

Make sure your Crownstone has been setup, microapps are only allowed in normal mode.

There are 3 different ways to upload the microapp:

- BLE: Over the air, handy when you want to upload a microapp to a live Crownstone.
- UART: Handy when you already have a UART connection to a Crownstone or development kit, as this uploads faster than via BLE.
- SWD: Quickest to upload and iterate, but requires a JLink and its dependencies. Does not interfere with UART debug logs.

## BLE

Make sure you have the [crownstone SDK](https://github.com/crownstone/crownstone-python-sdk) installed.

Then use:
```
make upload-ble
```

## UART

Make sure you have the [crownstone SDK](https://github.com/crownstone/crownstone-python-sdk) installed.

Then use:
```
make upload-uart
```

Note that if you are already running a UART log client, this will interfere with the upload.

## SWD

Uploading via SWD assumes you have the bluenet repository installed, and thus all the tools required for SWD flashing.

Make sure to build the firmware with `AUTO_ENABLE_MICROAPP_ON_BOOT=1` in the build config.

Then simply:
```
make flash
make reset
```
