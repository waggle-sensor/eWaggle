#include "waggle_common.h"
#include <cstdio>

namespace waggle {

class PrintfWriter : public Writer {
public:

    PrintfWriter(const char *fmt) : fmt(fmt) {
    }

    size_t Write(const byte *data, size_t size) {
        for (size_t i = 0; i < size; i++) {
            printf(fmt, (byte)(data[i]));
        }

        return size;
    }

private:

    const char *fmt;
};

};
