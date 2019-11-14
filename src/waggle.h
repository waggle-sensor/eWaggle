#ifndef __H_WAGGLE__
#define __H_WAGGLE__

#include <stdint.h>

typedef uint8_t byte;

// reader is the interface providing the basic read method.
struct reader {
  virtual int read(byte *s, int n) = 0;

  // readbyte is a convinient interface for single reads
  byte readbyte() {
    byte b;
    read(&b, 1);
    return b;
  }
};

// writer is the interface providing the basic write method.
struct writer {
  virtual int write(const byte *s, int n) = 0;

  // writebyte is a convinient interface for single writes
  void writebyte(byte b) { write(&b, 1); }
};

// closer is the interface providing the basic close method.
struct closer {
  virtual void close() = 0;
};

#include "waggle/base64.h"
#include "waggle/bytebuffer.h"
#include "waggle/sensorgram.h"

#endif
