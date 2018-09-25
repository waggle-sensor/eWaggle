#include "waggle/waggle_arduino.h"

using namespace waggle;

StreamIO systemIO(SerialUSB);
Messenger<4*1024> messenger(systemIO);

void setup() {
    SerialUSB.begin(9600);
    SerialUSB.setTimeout(1000);
}

void loop() {
    while (messenger.ReadMessage()) {
        messenger.WriteMessage(messenger.Message());
    }
}
