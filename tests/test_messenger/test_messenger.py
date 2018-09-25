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

        startTime = time.time()
        messenger.writeMessage(msg)
        resp = messenger.readMessage()
        duration = time.time() - startTime

        print(msg == resp, duration)
