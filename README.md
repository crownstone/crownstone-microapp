# Bluenet microapp

Create a microapp that can be run on top of the bluenet firmware on a Crownstone. Very first implementation.

# Organization

The files can be found in `include` and `src`. There is an example file in the root directory called `example.c`. The
results can be found in the `build` directory.

# Targets

Find out the targets through `make help`. Summarized, `make`, and `make flash` (to program the device). There are 
probably a lot more that are undocumented, for that inspect the `Makefile` yourself.

The usual way of development is:

```
make clean
make
make flash
make reset
```

This makes sure everything is uploaded fine and dandy. In the meantime you can also update the bluenet code. Make 
sure it is microapp compatible of course. You might encounter a checksum error if you do the above.

```
[t/source/src/cs_Crownstone.cpp : 271  ] ---- init microapp ----
[ce/src/storage/cs_MicroApp.cpp : 218  ] Sucessfully initialized from 0x00069000 to ...
[ce/src/storage/cs_MicroApp.cpp : 477  ] Micro app 0 has checksum 0x6620, but calculation shows 0xA07C
[ce/src/storage/cs_MicroApp.cpp : 241  ] Checksum error
```

This means that the bluenet code has still the checksum of the previous app and will not load this one. Enabling the
app will automatically correct the checksum. You can enable the app with something like:

```
scripts/microapp.py $PRIVATE_PATH/sphere-keys.json AA:BB:CC:DD:EE:FF build/example.bin enable
```

Make sure your sphere keys are not accessible and not accidentally committed to a code repository. When you run the 
bluenet code in debug mode you can find the Bluetooth MAC address of the device you are uploading to using UART. The above can also be done by:

```
make ota-enable
```

See below for how to set that up.

# Configuration

Create a `private.mk` file with a link to a file somewhere private (say `~/.crownstone/keys`) which contains your keys and the Crownstone with a particular Bluetooth address:

```
KEYS_JSON=~/.crownstone/keys/your_sphere_keys.json
BLE_ADDRESS=XX:XX:XX:XX:XX:XX
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

# Process

You can use the `microapp.py` script in the `scripts` directory to upload over the air.

1. Make sure you have firmware compiled with the `BUILD_MICROAPP_SUPPORT=1` option in `CMakeBuild.config`.
2. Make sure you have a `private.mk` file with up to date info (see above at "Configuration").
3. Adjust `config.mk` to paths for your local system. Alternatively, write those paths in `private.mk`.
4. Run `make` to build the application.
5. Run `make ota-upload` to upload the application (over serial, use `make flash`).
6. Run `make ota-validate` to validate (this sends a checksum).
7. Run `make ota-enable` to enable the application (this is up to a reboot).

Currently we do not yet allow a persistent enable option. However, everything is already supported in the hardware for
this. This is a choice.

# Todo

There are a few TODOs left.

* Implement all typical Arduino functions in the firmware.
* Implement proper handling of responses in the python libs.


