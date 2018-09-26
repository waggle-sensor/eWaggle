#include "waggle/waggle_native.h"

using namespace Waggle;

LoopbackIO<512> loopback;

void testPackDatagram() {
    Datagram<256> dg;
    Sensorgram<32> s;

    s.sensorID = 1;
    s.parameterID = 0;
    s.SetUint(1000);
    s.Pack(dg.Body());

    s.sensorID = 2;
    s.parameterID = 0;
    s.SetUint(341);
    s.Pack(dg.Body());

    PrintfWriter printer("%02x");
    dg.Pack(printer);
    dg.Pack(printer);
}

void testPack() {
    Sensorgram<32> s;

    s.sensorID = 1;
    s.parameterID = 0;
    s.SetUint(1000);
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
    s.SetUint(43);
    s.Pack(loopback);
}

void testUnpack() {
    Sensorgram<32> s;

    while (s.Unpack(loopback)) {
        printf("ok %d %d %d %u\n", s.sensorID, s.parameterID, s.Length(), s.GetUint());
    }
}

int main() {
    testPack();
    testUnpack();
    testPackDatagram();
}
