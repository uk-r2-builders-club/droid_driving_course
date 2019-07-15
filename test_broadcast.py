#!/usr/bin/python3

import broadcast
import sys

broadcast = broadcast.BroadCaster()
message = sys.argv[1]
b = bytearray()
b.extend(map(ord, message))

broadcast.broadcast_message(b)

