#include "waggle_common.h"

namespace waggle {

class StreamIO : public ReadWriter {
public:

    StreamIO(Stream &stream) : stream(stream) {
    }

    int Read(byte *data, int size) {
        return stream.readBytes(data, size);
    }

    int Write(const byte *data, int size) {
        return stream.write(data, size);
    }

private:

    Stream &stream;
};

};
