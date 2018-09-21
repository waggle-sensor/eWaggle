#include "waggle/waggle.h"
#include <cstdio>

using namespace waggle;

class PrintfWriter : public Writer {
public:

    PrintfWriter(const char *fmt) : fmt(fmt) {
    }

    int Write(const byte *data, int size) {
        for (int i = 0; i < size; i++) {
            printf(fmt, (byte)(data[i]));
        }

        return size;
    }

private:

    const char *fmt;
};

void testEncodeSensorgram() {
    printf("--- testEncodeSensorgram\n");

    PrintfWriter printer("%02x");
    Encoder encoder(printer);

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

    printf("sensorgram ");
    encoder.EncodeSensorgram(sensorgram1, (const byte *)"hello", 5);
    encoder.EncodeSensorgram(sensorgram2, (const byte *)"second", 6);
    printf("\n");
}

void testEncodeDatagram() {
    printf("--- testEncodeDatagram\n");

    PrintfWriter printer("%02x");
    Encoder encoder(printer);

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

    printf("datagram ");
    encoder.EncodeDatagram(datagram, (const byte *)"some data", 9);
    printf("\n");
}

void testPlugin() {
    printf("--- testPlugin\n");

    PrintfWriter printer("%02x");
    Plugin<256> plugin(37, 2, 0, 0, 0); // move into struct or name params...

    printf("publish ");

    plugin.AddMeasurement(1, 0, 0, 0, (byte *)"first", 5); // same thing...
    plugin.AddMeasurement(2, 0, 0, 0, (byte *)"second", 6);
    plugin.PublishMeasurements(printer);

    plugin.AddMeasurement(3, 0, 1, 0, (byte *)"333", 3);
    plugin.PublishMeasurements(printer);

    plugin.AddMeasurement(4, 3, 1, 0, (byte *)"4", 1);
    plugin.PublishMeasurements(printer);

    printf("\n");
}

void testMessageReceiver() {
    printf("--- testMessageReceiver\n");

    Buffer<256> buffer;
    MessageWriter msgWriter(buffer);
    MessageReader msgReader(buffer);

    msgWriter.WriteMessage((byte *)"hello world", 11);
    msgWriter.WriteMessage((byte *)"another", 7);

    Buffer<256> msg;
    PrintfWriter printer("%c");

    while (msgReader.ReadMessage(msg)) {
        printf("message ");
        Copy(msg, printer);
        printf("\n");
        msg.Reset();
    }
}

void testEncodeDecode() {
    printf("--- testEncodeDecode\n");

    Buffer<256> buffer;
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
        data[size] = 0;
        printf("%d %d %s\n", s.sensorID, s.parameterID, (const char *)data);
    }
}

void testWriteMessage() {
    printf("--- testWriteMessage\n");

    PrintfWriter printer("%02x");
    MessageWriter msgWriter(printer);

    printf("writeMessage ");
    msgWriter.WriteMessage((byte *)"first", 5);
    msgWriter.WriteMessage((byte *)"second", 6);
    msgWriter.WriteMessage((byte *)"third", 5);
    printf("\n");
}

void testMessenger() {
    printf("--- testMessenger\n");

    Buffer<256> loopbackBuffer;
    Messenger<256> messenger(loopbackBuffer);

    messenger.WriteMessage((byte *)"first", 5);
    messenger.WriteMessage((byte *)"second", 6);
    messenger.WriteMessage((byte *)"third", 5);

    PrintfWriter printer("%c");

    while (messenger.ReadMessage()) {
        printf("message \"");
        Copy(messenger.Message(), printer);
        printf("\"\n");
    }
}

void testComplete() {
    printf("--- testComplete\n");

    // setup loopback messenger
    Buffer<256> loopbackBuffer;
    Messenger<256> messenger(loopbackBuffer);

    // setup plugin
    Plugin<256> plugin(37, 2, 0, 0, 0);

    plugin.AddMeasurement(1, 0, 0, 0, (byte *)"first", 5);
    plugin.AddMeasurement(2, 0, 0, 0, (byte *)"second", 6);

    messenger.StartMessage();
    plugin.PublishMeasurements(messenger);
    messenger.EndMessage();

    plugin.AddMeasurement(3, 0, 1, 0, (byte *)"123", 3);
    plugin.AddMeasurement(4, 3, 1, 0, (byte *)"4", 1);

    messenger.StartMessage();
    plugin.PublishMeasurements(messenger);
    messenger.EndMessage();

    PrintfWriter printer("%02x");

    while (messenger.ReadMessage()) {
        printf("message \"");
        Copy(messenger.Message(), printer);
        printf("\"\n");
    }
}

// TODO ok...need to check max length case...

int main() {
    testEncodeSensorgram();
    testEncodeDatagram();
    testPlugin();
    testWriteMessage();
    testMessageReceiver();
    testEncodeDecode();
    testMessenger();
    testComplete();
}
