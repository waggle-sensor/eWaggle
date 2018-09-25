#!/usr/bin/env python3
import waggle.protocol
from waggle.messaging import Messenger
import sys
import re
import io
from contextlib import suppress
from pprint import pprint


def checkWriteMessage(data):
    m = Messenger(io.BytesIO(data))

    while True:
        msg = m.readMessage()

        if msg is None:
            break

        print('message', msg)


for line in sys.stdin:
    print(line)

    with suppress(AttributeError):
        s = re.match(r'sensorgram (\S+)', line).group(1)

        for r in waggle.protocol.unpack_sensorgrams(bytes.fromhex(s)):
            pprint(r)

        print()

    with suppress(AttributeError):
        s = re.match(r'datagram (\S+)', line).group(1)
        pprint(waggle.protocol.unpack_datagram(bytes.fromhex(s)))
        print()

    with suppress(AttributeError):
        s = re.match(r'publish (\S+)', line).group(1)

        for msg in waggle.protocol.unpack_datagrams(bytes.fromhex(s)):
            print('datagram header:')

            pprint(msg)
            print('sensor data:')

            for r in waggle.protocol.unpack_sensorgrams(msg['body']):
                pprint(r)

            print()

    with suppress(AttributeError):
        s = re.match(r'writeMessage (\S+)', line).group(1)
        checkWriteMessage(bytes.fromhex(s))
