namespace waggle {

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

class ReadWriter : public Reader, public Writer {
};

int Copy(Reader &r, Writer &w) {
    unsigned int total = 0;
    byte data[64];

    for (;;) {
        unsigned int n = r.Read(data, sizeof(data));
        w.Write(data, n);

        total += n;

        if (n < sizeof(data)) {
            break;
        }
    }

    return total;
}

class Array {
public:

    virtual const byte *Bytes() const = 0;
    virtual unsigned int Length() const = 0;
    virtual unsigned int Capacity() const = 0;
};

template<unsigned int N>
class Buffer : public ReadWriter, public Array {
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

    unsigned int Length() const {
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

};
