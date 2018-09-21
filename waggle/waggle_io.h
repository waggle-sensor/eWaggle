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

class Array {
public:

    virtual const byte *Bytes() const = 0;
    virtual size_t Length() const = 0;
    virtual size_t Capacity() const = 0;
};

template<size_t N>
class Buffer : public ReadWriter, public Array {
public:

    Buffer() {
        capacity = N;
        Reset();
    }

    Buffer(byte *data, size_t size) {
        capacity = N;
        Reset();
        Write(data, size);
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
        return buffer;
    }

    size_t Length() const {
        return length - offset;
    }

    size_t Capacity() const {
        return capacity;
    }

private:

    byte buffer[N];
    size_t capacity;
    size_t length;
    size_t offset;
};

size_t Copy(Reader &r, Writer &w) {
    size_t total = 0;
    byte buffer[64];

    for (;;) {
        size_t n = r.Read(buffer, sizeof(buffer));
        w.Write(buffer, n);

        total += n;

        if (n < sizeof(buffer)) {
            return total;
        }
    }
}

// size_t CopyN(Reader &r, Writer &w, size_t size) {
//     size_t total = 0;
//     byte buffer[64];
//
//     for (;;) {
//         //...
//         size_t n = r.Read(buffer, sizeof(data));
//         w.Write(buffer, n);
//
//         total += n;
//
//         if (n < sizeof(buffer)) {
//             return total;
//         }
//     }
// }

};
