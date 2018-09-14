#!/usr/bin/env python3
import waggle.protocol
import sys
import re
from contextlib import suppress
from pprint import pprint

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
