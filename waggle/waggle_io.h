namespace waggle {

typedef unsigned int size_t;

class Reader {
public:

    virtual size_t Read(byte *data, size_t size) = 0;
};

class Writer {
public:

    virtual size_t Write(const byte *data, size_t size) = 0;

    size_t WriteByte(byte b) {
        return Write(&b, 1);
    }
};

class ReadWriter : public Reader, public Writer {
};

class Buffer : public ReadWriter {
public:

    Buffer(byte *buf, int cap) {
        buffer = buf;
        capacity = cap;
        Reset();
    }

    size_t Read(byte *data, size_t size) {
        size_t i = 0;

        while (i < size && offset < length) {
            data[i++] = buffer[offset++];
        }

        return i;
    }

    size_t Write(const byte *data, size_t size) {
        size_t i = 0;

        while (i < size && length < capacity) {
            buffer[length++] = data[i++];
        }

        if (offset == length) {
            Reset();
        }

        return i;
    }

    void Reset() {
        length = 0;
        offset = 0;
    }

    const byte *Bytes() const {
        return &buffer[offset];
    }

    size_t Length() const {
        return length - offset;
    }

    size_t Capacity() const {
        return capacity;
    }

private:

    byte *buffer;
    size_t capacity;
    size_t length;
    size_t offset;
};

template<size_t N>
class LoopbackIO : public ReadWriter {
public:

    LoopbackIO() : buffer(bytes, N) {
    }

    size_t Write(const byte *data, size_t size) {
        return buffer.Write(data, size);
    }

    size_t Read(byte *data, size_t size) {
        return buffer.Read(data, size);
    }

private:

    byte bytes[N];
    Buffer buffer;
};

};
