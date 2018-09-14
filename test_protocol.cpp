#include "waggle.h"
#include <cstdio>

char encodeBuffer[1024];

void printHex(const char *data, int length) {
    for (int i = 0; i < length; i++) {
        printf("%02x", data[i] & 0xff);
    }
}

void testEncodeSensorgram() {
    waggle::Buffer buffer(encodeBuffer, 1024);

    waggle::Encoder encoder(buffer);
    waggle::SensorgramInfo s;

    s.sensorID = 1;
    s.parameterID = 0;
    s.timestamp = 1000000;
    encoder.EncodeSensorgram(s, "first", 5);

    s.sensorID = 2;
    s.parameterID = 0;
    s.timestamp = 1000000;
    encoder.EncodeSensorgram(s, "second", 6);

    printf("sensorgram ");
    printHex(buffer.Bytes(), buffer.Length());
    printf("\n");
}

void testEncodeDatagram() {
    waggle::Buffer buffer(encodeBuffer, 1024);
    waggle::Encoder encoder(buffer);
    waggle::DatagramInfo dg;

    dg.pluginID = 37;
    encoder.EncodeDatagram(dg, "some data", 8);

    printf("datagram ");
    printHex(buffer.Bytes(), buffer.Length());
    printf("\n");
}

// class Plugin {
// };

int main() {
    testEncodeSensorgram();
    testEncodeDatagram();
    //
    // plugin.clearMeasurements();
    // plugin.addMeasurement(...);
    // plugin.addMeasurement(...);
    // plugin.publishMeasurements();
}
//
// EncodeInt(1, dg.protocolVersion); // auto
// EncodeInt(1, dg.timestamp); // maybe auto??
// EncodeInt(2, dg.pluginID); // one time
// EncodeInt(4, dg.timestamp);
// EncodeInt(2, dg.packetSeq);
// EncodeInt(1, dg.packetType);
// EncodeInt(2, dg.pluginID); // TODO check the layout again.
// EncodeInt(1, dg.pluginMajorVersion);
// EncodeInt(1, dg.pluginMinorVersion);
// EncodeInt(1, dg.pluginPatchVersion);
// EncodeInt(1, dg.pluginInstance);
// EncodeInt(2, dg.pluginRunID);
