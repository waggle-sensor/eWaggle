namespace waggle {

typedef unsigned char byte;

const byte crc8table[256] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
    0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
    0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
    0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
    0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
    0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
    0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
    0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
    0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
    0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
    0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
    0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
    0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
    0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
    0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
    0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
    0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
    0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
    0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
    0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
    0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
    0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35,
};

byte crc8(const byte *data, int size) {
    byte crc = 0;

    for (int i = 0; i < size; i++) {
        crc = crc8table[crc ^ data[i]];
    }

    return crc;
}

class Reader {
public:

    virtual int Read(byte *data, int length) = 0;
};

// Writer Interface
class Writer {
public:

    virtual int Write(const byte *data, int size) = 0;

    int WriteByte(byte b) {
        return Write(&b, 1);
    }
};

int Copy(Reader &r, Writer &w) {
    int total = 0;
    byte data[256];

    for (;;) {
        int n = r.Read(data, sizeof(data));
        w.Write(data, n);

        total += n;

        if (n < sizeof(data)) {
            break;
        }
    }

    return total;
}

template<unsigned int N>
class Buffer : public Reader, public Writer {
public:

    Buffer() {
        capacity = N;
        Reset();
    }

    int Read(byte *data, int size) {
        int i = 0;

        while (i < size && offset < length) {
            data[i++] = buffer[offset++];
        }

        return i;
    }

    int Write(const byte *data, int size) {
        int i = 0;

        while (i < size && length < capacity) {
            buffer[length++] = data[i++];
        }

        return i;
    }

    void Reset() {
        length = 0;
        offset = 0;
    }

    const byte *Bytes() const {
        return buffer;
    }

    int Length() const {
        return length;
    }

    unsigned int Capacity() const {
        return capacity;
    }

private:

    byte buffer[N];
    unsigned int capacity;
    unsigned int length;
    unsigned int offset;
};

#ifdef Stream

class StreamWriter : public Writer {
public:

    StreamWriter(Stream &s) : stream(s) {
    }

    int Write(const byte *data, int size) {
        return s.write(data, size);
    }

private:

    Stream &stream;
};

#endif

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

    void ConsumeMeasurements() {
        //
    }

    void ProcessMeasurements(SensorgramInfo &info) {
        //
    }

private:

    DatagramInfo datagramInfo;
    Buffer<N> buffer;
};

};
