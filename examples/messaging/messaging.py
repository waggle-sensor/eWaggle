#!/usr/bin/env python3
import argparse
from serial import Serial
from waggle.messaging import Messenger
from waggle.protocol import pack_sensorgram
from waggle.protocol import unpack_sensorgram
import time

parser = argparse.ArgumentParser()
parser.add_argument('--timeout', type=float, default=5.0)
parser.add_argument('device')
parser.add_argument('baudrate', type=int)
args = parser.parse_args()


with Serial(args.device, baudrate=args.baudrate, timeout=args.timeout) as ser:
    messenger = Messenger(ser)

    while True:
        req = pack_sensorgram({
            'sensor_id': 1,
            'parameter_id': 0,
            'value': b'hello',
        })

        messenger.write_message(req)

        req = pack_sensorgram({
            'sensor_id': 2,
            'parameter_id': 0,
            'value': b'hello',
        })

        messenger.write_message(req)

        for resp in messenger.read_messages():
            print('response')
            print(unpack_sensorgram(resp))

        time.sleep(1)
