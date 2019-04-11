# Example of detecting and reading a block from a MiFare NFC card.
# Author: Tony DiCola & Roberto Laricchia
# Copyright (c) 2015 Adafruit Industries
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""
This example shows connecting to the PN532 and writing & reading an ntag2xx
type RFID tag
"""

import board
import busio
import binascii
import sys
import ndef
import requests
from digitalio import DigitalInOut
from adafruit_pn532.i2c import PN532_I2C

# I2C connection:
i2c = busio.I2C(board.SCL, board.SDA)

# Non-hardware reset/request with I2C
pn532 = PN532_I2C(i2c, debug=False)

ic, ver, rev, support = pn532.get_firmware_version()
print('Found PN532 with firmware version: {0}.{1}'.format(ver, rev))

# Configure PN532 to communicate with MiFare cards
pn532.SAM_configuration()

keym = bytes([ 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 ])
keya = bytes([ 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 ])
baseurl = "http://localhost:5000/"

print('Waiting for RFID/NFC card to write to!')
while True:
    # Check if a card is available to read
    uid = pn532.read_passive_target(timeout=0.25)
    # Try again if no card is available.
    if uid is None:
        continue

    print('Found card with UID:', [hex(i) for i in uid])

    # Authenticate block
    pn532.mifare_classic_authenticate_block(uid, 4, 0x60, keya)

    block = bytearray()
    data = pn532.mifare_classic_read_block(4)
    block.extend(data)
    data = pn532.mifare_classic_read_block(5)
    block.extend(data)
    data = pn532.mifare_classic_read_block(6)
    block.extend(data)

    print('All: 0x{0}'.format(binascii.hexlify(block[:64])))

    start = 0
    end = 0

    for x in range(0, 48):
        if block[x] == 0x03:
           print("Start of ndef: ", x)
           start = x + 2
           length =int(block[x+1])
           print("Length: ", length)
        if block[x] == 0xFE:
           print("End of ndef: ", x)
           end = x

    decoder = ndef.message_decoder(block[start:end])
    for x in decoder:
        print("Data: {0}".format(x.data))
    





#    droid, droid_uid, member, member_uid = x.text.split('\n')
#
#    if __debug__:
#        print("Registering member: ", member_uid)
#    url = baseurl + "member/" + member_uid
#    try:
#        r = requests.get(url)
#    except:
#        print("Fail....")
#
#    if __debug__:
#        print("Registering droid: ", droid_uid)
#    url = baseurl + "droid/" + droid_uid
#    try:
#        r = requests.get(url)
#    except:
#        print("Fail....")

