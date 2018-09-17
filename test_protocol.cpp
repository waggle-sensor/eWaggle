#include "waggle.h"
#include <cstdio>

using namespace waggle;

byte bufferBytes[1024];

void printBuffer(const char *name, const Buffer &buffer) {
    printf("%s ", name);

    for (int i = 0; i < buffer.Length(); i++) {
        printf("%02x", buffer.Bytes()[i] & 0xff);
    }

    printf("\n");
}

void testEncodeSensorgram() {
    Buffer buffer(bufferBytes, 1024);

    Encoder encoder(buffer);

    SensorgramInfo sensorgram = {
        .sensorID = 1,
        .parameterID = 0,
        .timestamp = 1000000,
    };

    encoder.EncodeSensorgram(sensorgram, (const byte *)"hello", 5);

    printBuffer("sensorgram", buffer);
}

void testEncodeDatagram() {
    Buffer buffer(bufferBytes, 1024);
    Encoder encoder(buffer);

    DatagramInfo datagram = {
        .pluginID = 37,
        .pluginInstance = 0,
        .pluginMajorVersion = 0,
        .pluginMinorVersion = 0,
        .pluginPatchVersion = 0,
        .pluginRunID = 0,
        .packetSeq = 0,
        .packetType = 0,
        .timestamp = 1234,
    };

    encoder.EncodeDatagram(datagram, (const byte *)"some data", 9);

    printBuffer("datagram", buffer);
}

byte publishBufferBytes[1024];

void testPlugin() {
    Buffer publishBuffer(publishBufferBytes, sizeof(publishBufferBytes));

    Buffer pluginBuffer(bufferBytes, sizeof(bufferBytes));
    Plugin plugin(37, 2, 0, 0, 0, pluginBuffer);

    plugin.AddMeasurement(1, 0, 0, 0, (byte *)"first", 5);
    plugin.AddMeasurement(2, 0, 0, 0, (byte *)"second", 6);
    plugin.PublishMeasurements(publishBuffer);

    plugin.AddMeasurement(3, 0, 1, 0, (byte *)"333", 3);
    plugin.PublishMeasurements(publishBuffer);

    plugin.AddMeasurement(4, 3, 1, 0, (byte *)"4", 1);
    plugin.PublishMeasurements(publishBuffer);

    printBuffer("publish", publishBuffer);
}

int main() {
    testEncodeSensorgram();
    testEncodeDatagram();
    testPlugin();
}
