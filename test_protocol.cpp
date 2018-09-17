#include "waggle.h"
#include <cstdio>

using namespace waggle;

byte encodeBuffer[1024];

void printBuffer(const char *name, const byte *data, int length) {
    printf("%s ", name);

    for (int i = 0; i < length; i++) {
        printf("%02x", data[i] & 0xff);
    }

    printf("\n");
}

void testEncodeSensorgram() {
    Buffer buffer(encodeBuffer, 1024);

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

    printBuffer("sensorgram", buffer.Bytes(), buffer.Length());
}

void testEncodeDatagram() {
    Buffer buffer(encodeBuffer, 1024);
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

    printBuffer("datagram", buffer.Bytes(), buffer.Length());
}

byte bufferBytes[1024];
byte publishBytes[1024];

void testPlugin() {
    Buffer pluginBuffer(bufferBytes, sizeof(bufferBytes));
    Plugin plugin(pluginBuffer);

    plugin.SetID(7);
    plugin.SetVersion(1, 0, 2);

    plugin.AddMeasurement(1, 0, 0, 0, (byte *)"first", 5);
    plugin.AddMeasurement(2, 0, 0, 0, (byte *)"second", 6);
    plugin.AddMeasurement(3, 0, 1, 0, (byte *)"333", 3);

    Buffer publishBuffer(publishBytes, sizeof(publishBytes));
    plugin.PublishMeasurements(publishBuffer);
    printBuffer("publish", publishBuffer.Bytes(), publishBuffer.Length());
}

int main() {
    testEncodeSensorgram();
    testEncodeDatagram();
    testPlugin();
}
