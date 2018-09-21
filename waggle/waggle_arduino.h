#include "waggle_common.h"

namespace waggle {

class StreamIO : public ReadWriter {
public:

    StreamIO(Stream &s) : stream(s) {
    }

    int Read(byte *data, int size) {
        return s.readBytes(data, size);
    }

    int Write(const byte *data, int size) {
        return s.write(data, size);
    }

private:

    Stream &stream;
};

};
