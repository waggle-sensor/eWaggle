#include "waggle.h"
#include <cstdio>

byte encodeBuffer[1024];

void printHex(const byte *data, int length) {
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
    encoder.EncodeSensorgram(s, (const byte *)"first", 5);

    s.sensorID = 2;
    s.parameterID = 0;
    s.timestamp = 1000000;
    encoder.EncodeSensorgram(s, (const byte *)"second", 6);

    printf("sensorgram ");
    printHex(buffer.Bytes(), buffer.Length());
    printf("\n");
}

void testEncodeDatagram() {
    waggle::Buffer buffer(encodeBuffer, 1024);
    waggle::Encoder encoder(buffer);
    waggle::DatagramInfo dg;

    dg.pluginID = 37;
    encoder.EncodeDatagram(dg, (const byte *)"some data", 8);

    printf("datagram ");
    printHex(buffer.Bytes(), buffer.Length());
    printf("\n");
}

// class Plugin {
// };

int main() {
    testEncodeSensorgram();
    testEncodeDatagram();
}
