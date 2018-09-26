#include "waggle_crc.h"
#include "waggle_io.h"

namespace waggle {

const byte startByte = 0x7e;
const byte endByte = 0x7f;
const byte escapeByte = 0x7d;
const byte escapeMask = 0x20;

class MessageWriter : public Writer {
public:

    MessageWriter(Writer &writer) : writer(writer) {
    }

    size_t Write(const byte *data, size_t size) {
        for (size_t i = 0; i < size; i++) {
            if (data[i] == startByte || data[i] == endByte || data[i] == escapeByte) {
                writer.WriteByte(escapeByte);
                writer.WriteByte(data[i] ^ escapeMask);
            } else {
                writer.WriteByte(data[i]);
            }
        }

        return size;
    }

    void StartMessage() {
        writer.WriteByte(startByte);
    }

    void EndMessage() {
        writer.WriteByte(endByte);
    }

    void WriteMessage(const byte *data, size_t size) {
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

template<size_t N>
class Messenger : public Writer {
public:

    Messenger(ReadWriter &rw) : reader(rw), writer(rw), buffer(buf, N) {
        hasMessage = false;
    }

    void WriteMessage(const byte *data, size_t size) {
        return writer.WriteMessage(data, size);
    }

    void WriteMessage(Buffer &b) {
        return writer.WriteMessage(b.Bytes(), b.Length());
    }

    void StartMessage() {
        writer.StartMessage();
    }

    void EndMessage() {
        writer.EndMessage();
    }

    size_t Write(const byte *data, size_t size) {
        return writer.Write(data, size);
    }

    size_t Write(Buffer &b) {
        return writer.Write(b.Bytes(), b.Length());
    }

    bool ReadMessage() {
        if (hasMessage) {
            buffer.Reset();
        }

        hasMessage = reader.ReadMessage(buffer);
        return hasMessage;
    }

    Buffer &Message() {
        return buffer;
    }

private:

    MessageReader reader;
    MessageWriter writer;

    byte buf[N];
    Buffer buffer;
    bool hasMessage;
};

struct SensorgramInfo {
    unsigned int dataSize;
    unsigned int sensorID;
    unsigned int sensorInstance;
    unsigned int parameterID;
    unsigned int timestamp;
    unsigned int dataType;
};

struct DatagramInfo {
    unsigned int bodySize;
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
    byte dataCRC;
};

class Decoder {
public:

    Decoder(Reader &r) : reader(r) {
        error = false;
    }

    int Error() const {
        return error;
    }

    void DecodeBytes(byte *data, size_t size) {
        if (error) {
            return;
        }

        error = reader.Read(data, size) != size;
    }

    void DecodeBuffer(Buffer &buffer, size_t size) {
        byte data[size];
        DecodeBytes(data, size);
        buffer.Write(data, size);
    }

    unsigned char DecodeUint8() {
        byte b[1];
        DecodeBytes(b, 1);
        return b[0];
    }

    unsigned short DecodeUint16() {
        byte b[2];
        DecodeBytes(b, 2);
        return (b[0] << 8) + b[1];
    }

    unsigned int DecodeUint24() {
        byte b[3];
        DecodeBytes(b, 3);
        return (b[0] << 16) + (b[1] << 8) + b[2];
    }

    unsigned int DecodeUint32() {
        byte b[4];
        DecodeBytes(b, 4);
        return (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];
    }

    unsigned int DecodeUint(unsigned int size) {
        byte data[size];
        DecodeBytes(data, size);

        if (error) {
            return 0;
        }

        unsigned int r = 0;

        for (size_t i = 0; i < size; i++) {
            r <<= 8;
            r |= data[i];
        }

        return r;
    }

    void DecodeSensorgramInfo(SensorgramInfo &info) {
        info.dataSize = DecodeUint(2);
        info.sensorID = DecodeUint(2);
        info.sensorInstance = DecodeUint(1);
        info.parameterID = DecodeUint(1);
        info.timestamp = DecodeUint(4);
        info.dataType = DecodeUint(1);
    }

    void DecodeSensorgram(SensorgramInfo &info, byte *data) {
        DecodeSensorgramInfo(info);
        DecodeBytes(data, info.dataSize);
    }

    void DecodeDatagram(DatagramInfo &info, Writer &w) {
        if (DecodeUint(1) != 0xaa) {
            error = true;
            return;
        }

        info.bodySize = DecodeUint(3);
        info.protocolVersion = DecodeUint(1);
        info.timestamp = DecodeUint(4);
        info.packetSeq = DecodeUint(2);
        info.packetType = DecodeUint(1);
        info.pluginID = DecodeUint(2);
        info.pluginMajorVersion = DecodeUint(1);
        info.pluginMinorVersion = DecodeUint(1);
        info.pluginPatchVersion = DecodeUint(1);
        info.pluginInstance = DecodeUint(1);
        info.pluginRunID = DecodeUint(2);

        // fix to CopyN later...
        byte crc = 0;
        for (size_t i = 0; i < info.bodySize; i++) {
            byte b;
            DecodeBytes(&b, 1);
            w.WriteByte(b);
            crc = waggle::crc8(&b, 1, crc);
        }

        info.dataCRC = DecodeUint(1);

        if (crc != info.dataCRC) {
            error = true;
            return;
        }

        if (DecodeUint(1) != 0x55) {
            error = true;
            return;
        }
    }

private:

    Reader &reader;
    bool error;
};

class Encoder {
public:

    Encoder(Writer &w) : writer(w) {
        error = false;
    }

    void EncodeBytes(const byte *data, size_t size) {
        if (error) {
            return;
        }

        error = writer.Write(data, size) != size;
    }

    void EncodeBuffer(Buffer &buffer) {
        EncodeBytes(buffer.Bytes(), buffer.Length());
    }

    template<typename T>
    void EncodeInt(size_t size, T x) {
        byte data[size];

        // TODO careful about signedness / this was a very bad bug!!!
        for (int i = size - 1; i >= 0; i--) {
            data[i] = (byte)(x & 0xff);
            x >>= 8;
        }

        EncodeBytes(data, size);
    }

    void EncodeSensorgramInfo(const SensorgramInfo &s) {
        EncodeInt(2, s.dataSize);
        EncodeInt(2, s.sensorID);
        EncodeInt(1, s.sensorInstance);
        EncodeInt(1, s.parameterID);
        EncodeInt(4, s.timestamp);
        EncodeInt(1, s.dataType);
    }

    void EncodeSensorgram(const SensorgramInfo &s, const byte *data) {
        EncodeInt(2, s.dataSize);
        EncodeInt(2, s.sensorID);
        EncodeInt(1, s.sensorInstance);
        EncodeInt(1, s.parameterID);
        EncodeInt(4, s.timestamp);
        EncodeInt(1, s.dataType);
        EncodeBytes(data, s.dataSize);
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
    bool error;
};

static const byte TYPE_BYTES = 0;
static const byte TYPE_STRING = 1;
static const byte TYPE_BOOL = 2;
static const byte TYPE_INT = 3;
static const byte TYPE_UINT = 4;

size_t UintSize(unsigned int x) {
    size_t n = 0;

    do {
        n++;
        x >>= 8;
    } while (x > 0);

    return n;
}

size_t strlen(const char *s) {
    size_t n = 0;

    while (s[n]) {
        n++;
    }

    return n;
}

template<size_t N>
class Sensorgram {
public:

    unsigned int sensorID;
    unsigned int sensorInstance;
    unsigned int parameterID;
    unsigned int valueType;
    unsigned long timestamp;

    Sensorgram() : buffer(body, N) {
        sensorID = 0;
        parameterID = 0;
        sensorInstance = 0;
        timestamp = 0;
        valueType = 0;
    }

    Buffer &Body() {
        return buffer;
    }

    size_t Length() const {
        return buffer.Length();
    }

    void SetBytes(const byte *data, size_t size) {
        valueType = TYPE_BYTES;
        buffer.Reset();
        buffer.Write(data, size);
    }

    void SetString(const char *str) {
        valueType = TYPE_STRING;
        buffer.Reset();
        buffer.Write((const byte *)str, strlen(str));
    }

    void SetUint(unsigned int value) {
        valueType = TYPE_UINT;
        Encoder encoder(buffer);
        buffer.Reset();
        encoder.EncodeInt(UintSize(value), value);
    }

    void SetBool(bool value) {
        valueType = TYPE_BOOL;
        Encoder encoder(buffer);
        buffer.Reset();
        encoder.EncodeInt(1, value);
    }

    const byte *GetBytes() {
        return (const byte *)buffer.Bytes();
    }

    const char *GetString() {
        return (const char *)buffer.Bytes();
    }

    unsigned int GetUint() {
        Decoder decoder(buffer);
        return decoder.DecodeUint(buffer.Length());
    }

    bool GetBool() {
        Decoder decoder(buffer);
        return decoder.DecodeUint(buffer.Length());
    }

    bool Pack(Writer &writer) {
        Encoder encoder(writer);
        encoder.EncodeInt(2, buffer.Length());
        encoder.EncodeInt(2, sensorID);
        encoder.EncodeInt(1, sensorInstance);
        encoder.EncodeInt(1, parameterID);
        encoder.EncodeInt(4, timestamp);
        encoder.EncodeInt(1, valueType);
        encoder.EncodeBuffer(buffer);
        return true;
    }

    bool Unpack(Reader &reader) {
        Decoder decoder(reader);

        unsigned int length = decoder.DecodeUint(2);
        sensorID = decoder.DecodeUint(2);
        sensorInstance = decoder.DecodeUint(1);
        parameterID = decoder.DecodeUint(1);
        timestamp = decoder.DecodeUint(4);
        valueType = decoder.DecodeUint(1);

        buffer.Reset();
        decoder.DecodeBuffer(buffer, length);

        if (valueType == TYPE_STRING) {
            buffer.WriteByte(0);
        }

        return !decoder.Error();
    }

private:

    Buffer buffer;
    byte body[N];
};

template<size_t N>
struct Datagram {

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
    byte dataCRC;
    byte body[N];
    Buffer buffer;

    Datagram() : buffer(body, N) {
    }

    Buffer &Body() {
        return buffer;
    }

    bool Pack(Writer &writer) {
        Encoder encoder(writer);

        unsigned int crc = waggle::crc8(buffer.Bytes(), buffer.Length());

        encoder.EncodeInt(1, 0xaa);
        encoder.EncodeInt(3, buffer.Length());
        encoder.EncodeInt(1, protocolVersion);
        encoder.EncodeInt(4, timestamp);
        encoder.EncodeInt(2, packetSeq);
        encoder.EncodeInt(1, packetType);
        encoder.EncodeInt(2, pluginID);
        encoder.EncodeInt(1, pluginMajorVersion);
        encoder.EncodeInt(1, pluginMinorVersion);
        encoder.EncodeInt(1, pluginPatchVersion);
        encoder.EncodeInt(1, pluginInstance);
        encoder.EncodeInt(2, pluginRunID);
        encoder.EncodeBuffer(buffer);
        encoder.EncodeInt(1, crc);
        encoder.EncodeInt(1, 0x55);

        buffer.Reset();

        return true;
    }

    bool Unpack(Reader &reader) {
        return true;
    }
};

unsigned long defaultGetTimestamp() {
    return 0;
}

template<size_t N>
class Plugin {
public:

    Plugin(int id, int majorVersion, int minorVersion, int patchVersion, int instance) : buffer(buf, N) {
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
        sg.dataSize = size;

        Encoder encoder(buffer);
        encoder.EncodeSensorgram(sg, data);
    }

    void PublishMeasurements(Writer &writer) {
        datagramInfo.packetType = 0;

        Encoder encoder(writer);
        encoder.EncodeDatagram(datagramInfo, buffer.Bytes(), buffer.Length());
        buffer.Reset();

        datagramInfo.packetSeq++;
    }

    template<size_t N2>
    void PublishMeasurements(Messenger<N2> &messenger) {
        datagramInfo.packetType = 0;

        Encoder encoder(messenger);

        messenger.StartMessage();
        encoder.EncodeDatagram(datagramInfo, buffer.Bytes(), buffer.Length());
        messenger.EndMessage();
        buffer.Reset();

        datagramInfo.packetSeq++;
    }

    void ClearMeasurements() {
        buffer.Reset();
    }

private:

    DatagramInfo datagramInfo;
    byte buf[N];
    Buffer buffer;
};

};
