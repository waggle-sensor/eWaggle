#include <Waggle.h>

using namespace Waggle;

int main() {
    // First we define a loopback device with 256 bytes of scratch space on the stack.
    //
    // Note: A LoopbackIO device is a helper object which "loops" writes back to
    // reads. After testing, other IO devices such as SerialIO can be used as a
    // drop-in replacement.
    LoopbackIO<256> io;

    // Now, we define a sensorgram with 64 bytes of scratch space on the stack.
    Sensorgram<64> s;

    // In this section, we pack a bunch of test sensorgrams into our loopback device.
    s.sensorID = 1;
    s.parameterID = 0;
    s.SetString("starting!");
    s.Pack(io);

    s.sensorID = 2;
    s.parameterID = 0;
    s.SetUint(341);
    s.Pack(io);

    s.sensorID = 3;
    s.parameterID = 0;
    s.sensorInstance = 1;
    s.SetInt(-12345);
    s.Pack(io);

    s.sensorID = 4;
    s.parameterID = 3;
    s.sensorInstance = 1;
    s.SetUint(9999);
    s.Pack(io);

    s.sensorID = 9;
    s.parameterID = 3;
    s.sensorInstance = 2;
    s.SetString("hello!");
    s.Pack(io);

    // Finally, we re-unpack the sensorgrams from our loopback device and print
    // some basic info about them.
    while (s.Unpack(io)) {
        printf("sensor id: %d\n", s.sensorID);
        printf("parameter id: %d\n", s.parameterID);

        if (s.valueType == TYPE_STRING) {
            printf("string value: %s\n", s.GetString());
        } else if (s.valueType == TYPE_UINT) {
            printf("uint value: %d\n", s.GetUint());
        } else if (s.valueType == TYPE_INT) {
            printf("int value: %d\n", s.GetInt());
        }

        printf("\n");
    }

    return 0;
}
