#!/usr/bin/env python3

"""An example that turns on a Crownstone with given MAC address."""

import time
import argparse

from crownstone_ble import CrownstoneBle

from crownstone_core.packets.MicroappPacket import MicroappUploadCmd, MicroappRequestCmd, MicroappValidateCmd, MicroappEnableCmd

parser = argparse.ArgumentParser(description='Microapp commands')
parser.add_argument('--hciIndex', dest='hciIndex', metavar='I', type=int, nargs='?', default=0,
        help='The hci-index of the BLE chip')
parser.add_argument('keyFile',
        help='The json file with key information, expected values: admin, member, guest, basic,' + 
        'serviceDataKey, localizationKey, meshApplicationKey, and meshNetworkKey')
parser.add_argument('bleAddress',
        help='The BLE address of Crownstone to switch')
parser.add_argument('microapp', 
        help='The microapp (.obj) file to be sent.')
parser.add_argument('action',
        help='The action to be done (request, upload, validate, all).')

args = parser.parse_args()

print("===========================================\n\nStarting Example\n\n===========================================")

# Each action can be executed individually or at once.
actions = set()
if args.action == 'request':
    actions.add('request')
if args.action == 'upload':
    actions.add('upload')
if args.action == 'validate':
    actions.add('validate')
if args.action == 'enable':
    actions.add('enable')
if args.action == 'disable':
    actions.add('disable')
if args.action == 'add':
    actions.add('request')
    actions.add('upload')
    actions.add('validate')
    actions.add('enable')


# Initialize the Bluetooth Core.
core = CrownstoneBle(hciIndex=args.hciIndex)
core.loadSettingsFromFile(args.keyFile);

print("Connecting to", args.bleAddress)

class connectionSettings(object):
    def __init__(self, mtu):
        self.mtu = mtu

#conSettings = connectionSettings(300)
#print("Set connection MTU to", conSettings.mtu)
ret = core.connect(args.bleAddress, False)
if ret == False:
    print("Connection failed (will not disconnect anymore then)")
    #core.control.disconnect()
    core.shutDown()
    quit()

print("Read microapp to be sent", args.microapp)
with open(args.microapp, "rb") as f:
    buf = f.read()

# Only support for a single app (for now)
app_id = 0

# We are at microapp protocol version 0. This version has to be supported by both the python lib and the firmware
# version at the Crownstone you are uploading to.
protocol = 0

# The chunk size should be the same as on the Crownstone. The chunk size depends on the MTU settings of the 
# firmware. It is therefore considered dynamic. We check if we use the proper chunk_size with a request.
chunk_size = 40

# Offset of dummy_main in executable (use nm, objdump, readelf, etc.)
#offset=0xB4

offset=0x0

if 'request' in actions:
    print('Request a new app upload')
    cmd = MicroappRequestCmd(protocol, app_id, buf, chunk_size)
    core.control.requestMicroapp(cmd)

if 'upload' in actions:
    print('Upload the data itself (this is a sequence of commands)')
    cmd = MicroappUploadCmd(protocol, app_id, buf, chunk_size)
    core.control.sendMicroapp(cmd)

if 'validate' in actions:
    print('Validate')
    cmd = MicroappValidateCmd(protocol, app_id, buf, chunk_size)
    core.control.validateMicroapp(cmd)

if 'enable' in actions:
    print('Enable the app')
    cmd = MicroappEnableCmd(protocol, app_id, True, offset)
    core.control.enableMicroapp(cmd)

if 'disable' in actions:
    print('Disable the app')
    cmd = MicroappEnableCmd(protocol, app_id, False, 0x00)
    core.control.enableMicroapp(cmd)

print("Make sure commands have been received, sleep")
time.sleep(4)

print("Disconnect")
core.control.disconnect()
core.shutDown()

print("===========================================\n\nFinished Example\n\n===========================================")

