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

const byte startByte = 0x7e;
const byte endByte = 0x7f;
const byte escapeByte = 0x7d;
const byte escapeMask = 0x20;

class MessageSender {
public:

    MessageSender(Writer &writer) : writer(writer) {
    }

    void SendMessage(const byte *data, int size) {
        writer.WriteByte(startByte);

        for (int i = 0; i < size; i++) {
            if (data[i] == startByte || data[i] == endByte || data[i] == escapeByte) {
                writer.WriteByte(escapeByte);
                writer.WriteByte(data[i] ^ escapeMask);
            } else {
                writer.WriteByte(data[i]);
            }
        }

        writer.WriteByte(endByte);
    }

private:

    Writer &writer;
};

class MessageReceiver {
public:

    MessageReceiver(Reader &reader) : reader(reader) {
        start = false;
        escape = false;
    }

    bool ReceiveMessage(Writer &writer) {
        byte b;

        while (!start) {
            if (reader.Read(&b, 1) == 0) {
                return false;
            }

            if (b == startByte) {
                start = true;
                escape = false;
            }
        }

        while (start) {
            if (reader.Read(&b, 1) == 0) {
                return false;
            }

            if (b == endByte) {
                start = false;
                return true;
            }

            if (escape) {
                writer.WriteByte(b ^ escapeMask);
                escape = false;
            } else if (b == escapeByte) {
                escape = true;
            } else {
                writer.WriteByte(b);
            }
        }

        return false;
    }

private:

    Reader &reader;
    bool start;
    bool escape;
};

class PrintWriter : public Writer {
public:

    int Write(const byte *data, int size) {
        for (int i = 0; i < size; i++) {
            printf("%02x ", (byte)(data[i]));
        }

        return size;
    }
};

int main() {
    // testEncodeSensorgram();
    // testEncodeDatagram();
    // testPlugin();

    PrintWriter printer;
    Buffer msgBuffer(bufferBytes, sizeof(bufferBytes));
    MessageSender sender(msgBuffer);

    const byte test[] = {0xaa, startByte, escapeByte, endByte, 0x55};
    sender.SendMessage(test, sizeof(test));

    printBuffer("msg", msgBuffer);

    msgBuffer.WriteByte(startByte);
    msgBuffer.WriteByte(1);
    msgBuffer.WriteByte(2);
    msgBuffer.WriteByte(3);

    MessageReceiver receiver(msgBuffer);

    while (receiver.ReceiveMessage(printer)) {
        printf("got message!\n");
    }

    // Buffer buffer(exampleBytes, sizeof(exampleBytes));
    // Encoder encoder(buffer);
    //
    // SensorgramInfo sensorgram1 = {
    //     .sensorID = 1,
    //     .parameterID = 3,
    //     .timestamp = 1000000,
    // };
    //
    // SensorgramInfo sensorgram2 = {
    //     .sensorID = 2,
    //     .parameterID = 7,
    //     .timestamp = 1000000,
    // };
    //
    // encoder.EncodeSensorgram(sensorgram1, (byte *)"hello", 5);
    // encoder.EncodeSensorgram(sensorgram2, (byte *)"data", 4);
    //
    // Decoder decoder(buffer);
    //
    // for (int i = 0; i < 2; i++) {
    //     SensorgramInfo s;
    //     byte data[64];
    //     int size;
    //
    //     decoder.DecodeSensorgram(s, data, size);
    //     data[size] = 0;
    //     printf("%d %d %s\n", s.sensorID, s.parameterID, (const char *)data);
    // }
}
