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

        void StartMessage() {
            writer.WriteByte(startByte);
        }

        void EndMessage() {
            writer.WriteByte(endByte);
        }

        size_t Write(const byte *data, size_t size) {
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

        Messenger(ReadWriter &rw) : buffer(buf, N), reader(rw), writer(rw) {
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

// we need to fix the fixed type thing too
// we should be able to pass buffer without templating the whole thing...

class Decoder {
public:

    Decoder(Reader &r) : reader(r) {
        error = false;
    }

    int Error() const {
        return error;
    }

    void DecodeBytes(byte *data, int size) {
        if (error) {
            return;
        }

        error = reader.Read(data, size) != size;
    }

    unsigned int DecodeUint(unsigned int size) {
        byte data[size];
        DecodeBytes(data, size);

        if (error) {
            return 0;
        }

        unsigned int r = 0;

        for (int i = 0; i < size; i++) {
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

    void EncodeBytes(const byte *data, int size) {
        if (error) {
            return;
        }

        error = writer.Write(data, size) != size;
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

template<size_t N>
class MessageScanner {
public:

    MessageScanner(Reader &reader) :sensorgramBuffer(buf, N), decoder(reader), sensorgramDecoder(sensorgramBuffer) {
    }

    bool ScanDatagram() {
        sensorgramBuffer.Reset();
        decoder.DecodeDatagram(datagramInfo, sensorgramBuffer);
        return !decoder.Error();
    }

    bool ScanSensorgrams() {
        sensorgramBuffer.Reset();
    }

    bool ScanSensorgram() {
        byte temp[32];
        sensorgramDecoder.DecodeSensorgram(sensorgramInfo, temp);
        return !sensorgramDecoder.Error();
    }

    const DatagramInfo &Datagram() const {
        return datagramInfo;
    }

    const SensorgramInfo &Sensorgram() const {
        return sensorgramInfo;
    }

    // need error support for things like full buffer, etc...

private:

    Decoder decoder;
    byte buf[N];
    Buffer sensorgramBuffer;
    Decoder sensorgramDecoder;
    DatagramInfo datagramInfo;
    SensorgramInfo sensorgramInfo;
};

};
