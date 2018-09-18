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

    SensorgramInfo sensorgram1 = {
        .sensorID = 1,
        .parameterID = 0,
        .timestamp = 1000000,
    };

    SensorgramInfo sensorgram2 = {
        .sensorID = 2,
        .parameterID = 1,
        .timestamp = 1000000,
    };

    encoder.EncodeSensorgram(sensorgram1, (const byte *)"hello", 5);
    encoder.EncodeSensorgram(sensorgram2, (const byte *)"second", 6);

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

    // expect all three datagrams appended with correct internal sensorgrams
    printBuffer("publish", publishBuffer);
}

byte exampleBytes[1024];

int main() {
    // testEncodeSensorgram();
    // testEncodeDatagram();
    // testPlugin();

    Buffer buffer(exampleBytes, sizeof(exampleBytes));
    Encoder encoder(buffer);

    SensorgramInfo sensorgram1 = {
        .sensorID = 1,
        .parameterID = 3,
        .timestamp = 1000000,
    };

    SensorgramInfo sensorgram2 = {
        .sensorID = 2,
        .parameterID = 7,
        .timestamp = 1000000,
    };

    encoder.EncodeSensorgram(sensorgram1, (byte *)"hello", 5);
    encoder.EncodeSensorgram(sensorgram2, (byte *)"data", 4);

    Decoder decoder(buffer);

    for (int i = 0; i < 2; i++) {
        SensorgramInfo s;
        byte data[64];
        int size;

        decoder.DecodeSensorgram(s, data, size);
        decoder.Decode
        data[size] = 0;
        printf("%d %d %s\n", s.sensorID, s.parameterID, (const char *)data);
    }
}
