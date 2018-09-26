#!/usr/bin/env python3
import argparse
from serial import Serial
from waggle.messaging import Messenger
import time
import secrets

parser = argparse.ArgumentParser()
parser.add_argument('--timeout', type=float, default=5.0)
parser.add_argument('device')
parser.add_argument('baudrate', type=int)
args = parser.parse_args()


with Serial(args.device, baudrate=args.baudrate, timeout=args.timeout) as ser:
    messenger = Messenger(ser)

    while True:
        msg = secrets.token_bytes(256)
        start_time = time.time()
        messenger.write_message(msg)
        resp = messenger.read_message()
        duration = time.time() - start_time
        print(msg == resp, duration)
