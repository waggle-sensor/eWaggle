#include "waggle.h"
#include <cstdio>

namespace waggle {

class Writer {
public:

    virtual int Write(const char *data, int size) = 0;

private:
};

void printHex(const char *data, int size) {
    for (int i = 0; i < size; i++) {
        printf("%02X ", data[i] & 0xff);
    }
}

class ConsoleWriter : public Writer {
public:

    int Write(const char *data, int size) {
        printHex(data, size);
        return size;
    }
};

class Buffer : public Writer {
public:

    Buffer(char *b, int c) : buffer(b), capacity(c) {
        Reset();
    }

    int Write(const char *data, int size) {

        for (int i = 0; i < size; i++) {
            if (length >= capacity) {
                return i;
            }

            buffer[length++] = data[i];
        }

        return size;
    }

    void Reset() {
        length = 0;
    }

    int Length() const {
        return length;
    }

private:

    char *buffer;
    int capacity;
    int length;
};

struct SensorgramInfo {
    int sensorID;
    int sensorInstance;
    int parameterID;
    long timestamp;
};

struct DatagramInfo {
    int protocolVersion;
    int timestamp;
    int packetSeq;
    int packetType;
    int pluginID;
    int pluginMajorVersion;
    int pluginMinorVersion;
    int pluginPatchVersion;
    int pluginInstance;
    int pluginRunID;
};

class Encoder {
public:

    Encoder(Writer &w) : writer(w) {
    }

    void EncodeBytes(const char *data, int size) {
        writer.Write(data, size);
    }

    void EncodeInt(int size, int x) {
        char data[size];

        for (int i = size - 1; i >= 0; i--) {
            data[i] = (char)(x & 0xff);
            x >>= 8;
        }

        EncodeBytes(data, size);
    }

    void EncodeSensorgram(const SensorgramInfo &s, const char *data, int size) {
        EncodeInt(2, size);
        EncodeInt(2, s.sensorID);
        EncodeInt(1, s.sensorInstance);
        EncodeInt(1, s.parameterID);
        EncodeInt(4, s.timestamp);
        EncodeBytes(data, size);
    }

    void EncodeDatagram(const DatagramInfo &dg, const char *body, int size) {
        EncodeInt(1, 0xaa);
        EncodeInt(3, size);
        EncodeInt(1, dg.protocolVersion);
        EncodeInt(1, dg.timestamp);
        EncodeInt(2, dg.pluginID);
        EncodeInt(4, dg.timestamp);
        EncodeInt(2, dg.packetSeq);
        EncodeInt(1, dg.packetType);
        EncodeInt(2, dg.pluginID);
        EncodeInt(1, dg.pluginMajorVersion);
        EncodeInt(1, dg.pluginMinorVersion);
        EncodeInt(1, dg.pluginPatchVersion);
        EncodeInt(1, dg.pluginInstance);
        EncodeInt(2, dg.pluginRunID);
        EncodeBytes(body, size);
        EncodeInt(1, 0); // Compute CRC
        EncodeInt(1, 0x55);
    }

private:

    Writer &writer;
};

};

int main() {
    char testBuffer[1024];
    waggle::Buffer w(testBuffer, 1024);

    waggle::Encoder enc(w);

    waggle::SensorgramInfo s;

    s.sensorID = 32;
    s.parameterID = 0;
    s.timestamp = 1043123;
    enc.EncodeSensorgram(s, "askeqwe", 1);
    enc.EncodeSensorgram(s, "askeqwe", 2);

    printHex(testBuffer, w.Length());
}
