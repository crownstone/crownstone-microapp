# Bluenet microapp

A microapp is like an addon for a Crownstone.
The goal of microapps is to make it easy to run additional code on top of released bluenet firmware.

Be careful what microapp you upload though. While there are built in safety measures, there are always ways to make your crownstone unaccassible with a microapp. It's best to first test a microapp on a development kit.

# Organization

The files that form the backbone of microapps can be found in `include` and `src`.
There are a lot of examples in `examples`.
The results of a build can be found in the `build` directory.
More details can be found in the `docs` dir.

# Writing a microapp

Microapps are written with Arduino-like syntax.
The `setup()` function is always executed first, and only once.
After that, the `loop()` function is called continously.

Have a look at the examples to quickly see what possibilities there are.

## Caution
Writing a good microapp is your responsibility, so always make sure to test it thouroughly.

There are some easy to make mistakes:

#### Permanent BLE connection
Bluenet currently only supports a single BLE connection. If the microapp stays connected continuously, there is no way anymore to send a control command to a Crownstone.


## Limitations

There are many limitations to microapps. The most important ones are listed here.

#### Throttling
Only a limited number of calls to bluenet are allowed per unit of time (tick). When this limit is reached, bluenet will automatically pause the execution of the microapp and continue the next tick.
If you want to make sure calls happen in the same tick, for example 3 digital writes for an RGB LED, this can be reached by adding a `delay()` before those calls.

The same goes for interrupts: only a limited number of interrupts per tick will reach the microapp. When this limit is reached, new interrupts within this tick will be dropped. This limit is implemented per type, so that interrupts of a certain type (for example BLE scans) will not lead to dropping interrupts of another type (for example a button press).

#### BLE peripheral and vendor specific UUIDs
When your microapp registered a BLE service, or uses custom UUIDs, the Crownstone will have to be reset in order to remove those again, in case you upload a new microapp.

#### RAM usage
While there is quite some RAM reserved for a microapp, a large portion of it is margin because (real) interrupts of bluenet use the microapp stack when they happen in microapp context (e.g. while the microapp is executing). When designing the microapp, make sure to keep 1kB margin.

#### No dynamic memory allocation
Dynamic memory allocation is not supported. This means no malloc, calloc, etc.
This includes std::vector and the likes.

#### No persistent storage
Persistent storage (flash storage) is not implemented yet.



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

# License

## Open-source license

This software is provided under a noncontagious open-source license towards the open-source community. It's available under three open-source licenses:
 
* License: LGPL v3+, Apache, MIT

<p align="center">
  <a href="http://www.gnu.org/licenses/lgpl-3.0">
    <img src="https://img.shields.io/badge/License-LGPL%20v3-blue.svg" alt="License: LGPL v3" />
  </a>
  <a href="https://opensource.org/licenses/MIT">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License: MIT" />
  </a>
  <a href="https://opensource.org/licenses/Apache-2.0">
    <img src="https://img.shields.io/badge/License-Apache%202.0-blue.svg" alt="License: Apache 2.0" />
  </a>
</p>

## Commercial license

This software can also be provided under a commercial license. If you are not an open-source developer or are not planning to release adaptations to the code under one or multiple of the mentioned licenses, contact us to obtain a commercial license.

* License: Crownstone commercial license

# Contact

For any question contact us at <https://crownstone.rocks/contact/> or on our discord server through <https://crownstone.rocks/forum/>.
