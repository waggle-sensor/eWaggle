#include "waggle_crc.h"
#include "waggle_io.h"

namespace waggle {

struct SensorgramInfo {
    unsigned int sensorID;
    unsigned int sensorInstance;
    unsigned int parameterID;
    unsigned int timestamp;
    unsigned int dataType;
};

struct DatagramInfo {
    unsigned int protocolVersion;
    unsigned int timestamp;
    unsigned int packetSeq;
    unsigned int packetType;
    unsigned int pluginID;
    unsigned int pluginMajorVersion;
    unsigned int pluginMinorVersion;
    unsigned int pluginPatchVersion;
    unsigned int pluginInstance;
    unsigned int pluginRunID;
};

class Decoder {
public:

    Decoder(Reader &r) : reader(r) {
    }

    int DecodeBytes(byte *data, int size) {
        return reader.Read(data, size);
    }

    template<typename T>
    int DecodeInt(int size, T &x) {
        byte data[size];

        int n = DecodeBytes(data, size);

        x = 0;

        for (int i = 0; i < n; i++) {
            x <<= 8;
            x |= data[i];
        }

        return n;
    }

    void DecodeSensorgram(SensorgramInfo &s, byte *data, int &size) {
        DecodeInt(2, size);
        DecodeInt(2, s.sensorID);
        DecodeInt(1, s.sensorInstance);
        DecodeInt(1, s.parameterID);
        DecodeInt(4, s.timestamp);
        DecodeInt(1, s.dataType);
        DecodeBytes(data, size);
    }

private:

    Reader &reader;
};

class Encoder {
public:

    Encoder(Writer &w) : writer(w) {
    }

    int EncodeBytes(const byte *data, int size) {
        return writer.Write(data, size);
    }

    template<typename T>
    void EncodeInt(int size, T x) {
        byte data[size];

        for (int i = size - 1; i >= 0; i--) {
            data[i] = (byte)(x & 0xff);
            x >>= 8;
        }

        EncodeBytes(data, size);
    }

    void EncodeSensorgram(const SensorgramInfo &s, const byte *data, int size) {
        EncodeInt(2, size);
        EncodeInt(2, s.sensorID);
        EncodeInt(1, s.sensorInstance);
        EncodeInt(1, s.parameterID);
        EncodeInt(4, s.timestamp);
        EncodeInt(1, s.dataType);
        EncodeBytes(data, size);
    }

    void EncodeDatagram(const DatagramInfo &dg, const byte *body, int size) {
        unsigned int crc = waggle::crc8(body, size);

        EncodeInt(1, 0xaa);
        EncodeInt(3, size);
        EncodeInt(1, dg.protocolVersion);
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
        EncodeInt(1, crc);
        EncodeInt(1, 0x55);
    }

private:

    Writer &writer;
};

unsigned long defaultGetTimestamp() {
    return 0;
}

template<unsigned int N>
class Plugin {
public:

    Plugin(int id, int majorVersion, int minorVersion, int patchVersion, int instance) {
        datagramInfo.protocolVersion = 2;

        datagramInfo.pluginID = id;
        datagramInfo.pluginMajorVersion = majorVersion;
        datagramInfo.pluginMinorVersion = minorVersion;
        datagramInfo.pluginPatchVersion = patchVersion;
        datagramInfo.pluginInstance = instance;

        datagramInfo.packetType = 0;
        datagramInfo.packetSeq = 0;
        datagramInfo.timestamp = 0;
        datagramInfo.pluginRunID = 0;
    }

    void AddMeasurement(int sid, int sinst, int pid, int type, byte *data, int size) {
        SensorgramInfo sg;

        sg.sensorID = sid;
        sg.sensorInstance = sinst;
        sg.parameterID = pid;
        sg.timestamp = defaultGetTimestamp();
        sg.dataType = type;

        Encoder encoder(buffer);
        encoder.EncodeSensorgram(sg, data, size);
    }

    void PublishMeasurements(Writer &writer) {
        datagramInfo.packetType = 0;

        Encoder encoder(writer);
        encoder.EncodeDatagram(datagramInfo, buffer.Bytes(), buffer.Length());
        buffer.Reset();

        datagramInfo.packetSeq++;
    }

    void ClearMeasurements() {
        buffer.Reset();
    }

private:

    DatagramInfo datagramInfo;
    Buffer<N> buffer;
};

class MessageScanner {
public:

    MessageScanner(Reader &reader) : reader(reader) {
    }

    bool Scan() {
        return true;
    }


private:

    Reader &reader;
};

const byte startByte = 0x7e;
const byte endByte = 0x7f;
const byte escapeByte = 0x7d;
const byte escapeMask = 0x20;

class MessageWriter : public Writer {
public:

    MessageWriter(Writer &writer) : writer(writer) {
    }

    void StartMessage() {
        writer.WriteByte(startByte);
    }

    void EndMessage() {
        writer.WriteByte(endByte);
    }

    int Write(const byte *data, int size) {
        for (int i = 0; i < size; i++) {
            if (data[i] == startByte || data[i] == endByte || data[i] == escapeByte) {
                writer.WriteByte(escapeByte);
                writer.WriteByte(data[i] ^ escapeMask);
            } else {
                writer.WriteByte(data[i]);
            }
        }

        return size;
    }

    void WriteMessage(const byte *data, int size) {
        StartMessage();
        Write(data, size);
        EndMessage();
    }

private:

    Writer &writer;
};

class MessageReader {
public:

    MessageReader(Reader &reader) : reader(reader) {
        start = false;
        escape = false;
    }

    bool ReadMessage(Writer &writer) {
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

template<unsigned int N>
class Messenger : public Writer {
public:

    Messenger(ReadWriter &rw) : reader(rw), writer(rw) {
        hasMessage = false;
    }

    void WriteMessage(const byte *data, int size) {
        return writer.WriteMessage(data, size);
    }

    void WriteMessage(const Array &a) {
        return writer.WriteMessage(a.Bytes(), a.Length());
    }

    void StartMessage() {
        writer.StartMessage();
    }

    void EndMessage() {
        writer.EndMessage();
    }

    int Write(const byte *data, int size) {
        return writer.Write(data, size);
    }

    int Write(const Array &a) {
        return writer.Write(a.Bytes(), a.Length());
    }

    bool ReadMessage() {
        if (hasMessage) {
            buffer.Reset();
        }

        hasMessage = reader.ReadMessage(buffer);
        return hasMessage;
    }

    Buffer<N> &Message() {
        return buffer;
    }

private:

    MessageReader reader;
    MessageWriter writer;
    Buffer<N> buffer;
    bool hasMessage;
};

};
