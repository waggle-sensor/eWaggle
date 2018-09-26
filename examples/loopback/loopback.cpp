#include "eWaggle.h"

using namespace waggle;

int main() {
    LoopbackIO<1024> loopback;
    Sensorgram<64> s;

    s.sensorID = 1;
    s.parameterID = 0;
    s.SetString("starting!");
    s.Pack(loopback);

    s.sensorID = 2;
    s.parameterID = 0;
    s.SetUint(341);
    s.Pack(loopback);

    s.sensorID = 3;
    s.parameterID = 0;
    s.sensorInstance = 1;
    s.SetUint(123);
    s.Pack(loopback);

    s.sensorID = 4;
    s.parameterID = 3;
    s.sensorInstance = 1;
    s.SetUint(9999);
    s.Pack(loopback);

    s.sensorID = 9;
    s.parameterID = 3;
    s.sensorInstance = 2;
    s.SetString("hello!");
    s.Pack(loopback);

    while (s.Unpack(loopback)) {
        printf("sensor id: %d\n", s.sensorID);
        printf("parameter id: %d\n", s.parameterID);

        if (s.valueType == TYPE_STRING) {
            printf("string value: %s\n", s.GetString());
        } else if (s.valueType == TYPE_UINT) {
            printf("uint value: %d\n", s.GetUint());
        }

        printf("\n");
    }

    return 0;
}
