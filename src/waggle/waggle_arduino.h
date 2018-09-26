#include "waggle_common.h"

namespace waggle {

class StreamIO : public ReadWriter {
public:

    StreamIO(Stream &stream) : stream(stream) {
    }

    size_t Read(byte *data, size_t size) {
        return stream.readBytes(data, size);
    }

    size_t Write(const byte *data, size_t size) {
        return stream.write(data, size);
    }

private:

    Stream &stream;
};

};
