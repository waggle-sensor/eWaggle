#include "waggle_common.h"
#include <cstdio>

namespace waggle {

class PrintfWriter : public Writer {
public:

    PrintfWriter(const char *fmt) : fmt(fmt) {
    }

    int Write(const byte *data, int size) {
        for (int i = 0; i < size; i++) {
            printf(fmt, (byte)(data[i]));
        }

        return size;
    }

private:

    const char *fmt;
};

};
