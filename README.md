# Bluenet module

Create a module that can be run from Crownstone code. Very first implementation.

# Organization

The files can be found in `include` and `src`. There is an example file in the root directory called `example.c`. The
results can be found in the `build` directory.

# Targets

Find out the targets through `make help`. Summarized, `make`, and `make flash` (to program the device). There are 
probably a lot more that are undocumented, for that inspect the `Makefile` yourself.

# Configuration

Create a `private.mk` file with a link to a file somewhere private (say `~/.crownstone/keys`) which contains your keys and the Crownstone with a particular Bluetooth address:

```
KEYS_JSON=~/.crownstone/keys/your_sphere_keys.json
BLE_ADDRESS=XX:XX:XX:XX:XX:XX
```

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


