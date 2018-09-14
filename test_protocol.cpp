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
    dg.pluginInstance = 0;
    dg.pluginMajorVersion = 0;
    dg.pluginMinorVersion = 0;
    dg.pluginPatchVersion = 0;
    dg.pluginRunID = 0;
    dg.packetSeq = 0;
    dg.packetType = 0;
    dg.timestamp = 1234;
    encoder.EncodeDatagram(dg, (const byte *)"some data", 9);

    printf("datagram ");
    printHex(buffer.Bytes(), buffer.Length());
    printf("\n");
}

byte publishBuffer[1024];

void testPlugin() {
    waggle::Buffer buffer(publishBuffer, 1024);
    waggle::Plugin plugin;

    plugin.AddMeasurement(1, 0, 0, 0, 0, (byte *)"some data", 9);
    plugin.PublishMeasurements(buffer);

    printf("publish ");
    printHex(buffer.Bytes(), buffer.Length());
    printf("\n");
}

int main() {
    testEncodeSensorgram();
    testEncodeDatagram();
    testPlugin();
}
