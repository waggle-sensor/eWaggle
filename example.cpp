#include "waggle/waggle_native.h"

using namespace waggle;

LoopbackIO<512> loopback;
Messenger<256> messenger(loopback);

void testPack() {
    Sensorgram<32> s;

    messenger.StartMessage();

    s.sensorID = 1;
    s.parameterID = 0;
    s.SetUint(1, 39);
    s.Pack(messenger);

    s.sensorID = 2;
    s.parameterID = 0;
    s.Pack(messenger);

    s.sensorID = 3;
    s.parameterID = 0;
    s.sensorInstance = 1;
    s.Pack(messenger);

    s.sensorID = 4;
    s.parameterID = 3;
    s.sensorInstance = 1;
    s.Pack(messenger);

    s.sensorID = 9;
    s.parameterID = 3;
    s.sensorInstance = 2;
    s.SetUint(1, 43);
    s.Pack(messenger);

    messenger.EndMessage();
}

void testUnpack() {
    Sensorgram<32> s;

    while (messenger.ReadMessage()) {
        while (s.Unpack(messenger.Message())) {
            printf("ok %d %d %d %u\n", s.sensorID, s.parameterID, s.Length(), s.GetUint(1));
        }
    }
}

int main() {
    testPack();
    testUnpack();
}
