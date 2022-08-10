from time import sleep

import nfc
import ndef
from nfc.clf import RemoteTarget

def parseNDEF():
    with nfc.ContactlessFrontend("usb:072f:2200") as clf:
        while True:
            target = clf.sense(RemoteTarget("106A"), RemoteTarget("106B"),
            RemoteTarget("212F"))



clf = nfc.ContactlessFrontend('usb:072f:2200')
target = clf.sense(RemoteTarget('106B'), RemoteTarget('106A'), RemoteTarget('212F'))
tag = nfc.tag.activate(clf, target)
print(tag)
tag = clf.connect(rdwr={'on-connect': lambda tag: False})

for record in tag.ndef.records:
    print(record)

