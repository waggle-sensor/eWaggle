# Loopback Example

This example shows how to to pack and unpack sensorgrams to a loopback IO
device.

## Organization

* [loopback.cpp](loopback.cpp) - Main source file.

## Running

To run the example, just run:

```sh
make test
```

You should see something like:

```sh
sensor id: 1
parameter id: 0
string value: starting!

sensor id: 2
parameter id: 0
uint value: 341

sensor id: 3
parameter id: 0
uint value: 123

sensor id: 4
parameter id: 3
uint value: 9999

sensor id: 9
parameter id: 3
string value: hello!
```
