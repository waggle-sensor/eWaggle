# eWaggle Module

eWaggle is a module aimed at Arduino-like systems which provides simple interfaces
for interacting with the Waggle platform.

It provides functionality in a few core area:

* Protocols - Interfaces for working with Waggle data formats such as sensorgrams and datagrams.
* Messaging - Interfaces for sending and receiving message frames over serial ports.
* Development - Helper objects to facilitate testing and debugging. Supports native builds for local testing.

## Installation

### Installation as Arduino Package

1. Download .ZIP of Repo - From Github, download this repo as a ZIP from the "Clone or download" button.

2. Install .ZIP in Arduino IDE - You can install the .ZIP file directly in the Arduino IDE using "Sketch -> Include Library -> Add .ZIP Library".

### Manual Installation

Clone the `eWaggle` repo and add `eWaggle/src` to your project's include path.
For example, your project's build process should have something like:

```make
CC=c++ -I/path/to/eWaggle/src ...
```

## Examples

Examples can be found in the [examples directory](./examples).
