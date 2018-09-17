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
    SensorgramInfo s;

    s.sensorID = 1;
    s.parameterID = 0;
    s.timestamp = 1000000;
    encoder.EncodeSensorgram(s, (const byte *)"first", 5);

    s.sensorID = 2;
    s.parameterID = 0;
    s.timestamp = 1000000;
    encoder.EncodeSensorgram(s, (const byte *)"second", 6);

    printBuffer("sensorgram", buffer);
}

void testEncodeDatagram() {
    Buffer buffer(bufferBytes, 1024);
    Encoder encoder(buffer);
    DatagramInfo dg;

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

    printBuffer("datagram", buffer);
}

byte publishBufferBytes[1024];

void testPlugin() {
    Buffer publishBuffer(publishBufferBytes, sizeof(publishBufferBytes));

    Buffer pluginBuffer(bufferBytes, sizeof(bufferBytes));
    Plugin plugin(1, 2, 0, 0, 0, pluginBuffer);

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
