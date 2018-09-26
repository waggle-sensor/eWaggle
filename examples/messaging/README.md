# Messaging Example

This examples shows how to use the basic messaging API by implementing a basic
request and response system between an Arduino and another machine over a serial
port.

```
+--------------+        +---------+
| Host Machine |        | Arduino |
|              | Serial | Client  |
|    [req] >------->------->      |
|              |        |  (proc) |
|   [resp] <-------<-------<      |
+--------------+        +---------+
```

## Files

* [messaging.ino](messaging.ino) - Arduino client source code.
* [messaging.py](messaging.py) - Host machine source code. Uses [pywaggle](https://github.com/waggle-sensor/pywaggle).

## Setup

The `messaging.ino` file should work with the standard [Arduino IDE](https://www.arduino.cc/en/Main/Software). By default, we're assuming the
target device is an Arduino Due. You'll need to copy this repos `waggle`
directory in the project folder to build.

To run `messaging.py`, you'll need the [pywaggle module](https://github.com/waggle-sensor/pywaggle)
to be installed to run `messaging.py`.
