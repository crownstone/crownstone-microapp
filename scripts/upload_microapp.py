#!/usr/bin/env python3

""" Experimental tool to upload a microapp to a Crownstone. """

import asyncio
import logging
from os import path
import datetime

from crownstone_ble import CrownstoneBle, BleEventBus, BleTopics
from util.config import getToolConfig, loadKeysFromConfig, setupDefaultCommandLineArguments, macFilterPassed

tool_version = "1.0.0"

parser = setupDefaultCommandLineArguments('Scan for any Crownstones continuously and print the results.')
parser.add_argument('-a', '--bleAddress', required=True, help='The MAC address/handle of the Crownstone you want to connect to')
parser.add_argument('-f', '--file', default=None, help='Microapp binary to upload')
parser.add_argument('-v', '--verbose', default=False,
                    help='Verbose will show the full advertisement content, not just a single line summary.')

# logging.basicConfig(format='%(asctime)s %(levelname)-7s: %(message)s', level=logging.DEBUG)

try:
    file_path = path.dirname(path.realpath(__file__))
    [tool_config, args] = getToolConfig(file_path, parser)
except Exception as e:
    print("ERROR", e)
    quit()

# create the library instance
print(f'Initializing tool with bleAdapterAddress={tool_config["bleAdapterAddress"]}')
core = CrownstoneBle(bleAdapterAddress=tool_config["bleAdapterAddress"])

# load the encryption keys into the library
try:
    loadKeysFromConfig(core, tool_config)
except Exception as e:
    print("ERROR", e)
    quit()


# It looks like the python library can't handle a chunk size of 256.
maxChunkSize = 192

# The index where we want to put our microapp.
appIndex = 0

async def main():
    print("Main")
    with open(args.file, "rb") as f:
        appData = f.read()

    print("First 32 bytes of the binary:")
    print(list(appData[0:32]))

    await core.connect(args.bleAddress)
    info = await core._dev.getMicroappInfo()
    print(info)

    # Perform some checks with the info we received.
    if appIndex >= info.maxApps:
        print(f"This crownstone doesn't have room for index {appIndex}")
        await core.disconnect()
        await core.shutDown()
        return

    if len(appData) > info.maxAppSize:
        print(f"This crownstone doesn't have room for a binary size of {len(appData)}")
        await core.disconnect()
        await core.shutDown()
        return

    # If there is already some data at this index, it has to be removed first.
    if info.appsStatus[appIndex].tests.hasData:
        print(f"Remove data at index {appIndex}")
        await core._dev.removeMicroapp(appIndex)

    # Determine the chunk size by taking the minimum of our max, and the crownstones max.
    chunkSize = min(maxChunkSize, info.maxChunkSize)

    print(f"{datetime.datetime.now()} Start upload with chunkSize={chunkSize}")
    await core._dev.uploadMicroapp(appData, appIndex, chunkSize)
    print(f"{datetime.datetime.now()} Upload done")

    print("Validate..")
    await core._dev.validateMicroapp(appIndex)
    print("Validate done")

    print("Enable..")
    await core._dev.enableMicroapp(appIndex)
    print("Enable done")

    await core.disconnect()
    await core.shutDown()

try:
    # asyncio.run does not work here.
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
except KeyboardInterrupt:
    print("Stopping.")
