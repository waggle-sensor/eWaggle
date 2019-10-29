#ifndef __H_WAGGLE__
#define __H_WAGGLE__

typedef unsigned char byte;

// reader is the interface providing the basic read method.
struct reader {
  virtual int read(byte *s, int n) = 0;

  byte readbyte() {
    byte b;
    read(&b, 1);
    return b;
  }
};

// writer is the interface providing the basic write method.
struct writer {
  virtual int write(const byte *s, int n) = 0;

  void writebyte(byte b) { write(&b, 1); }
};

// closer is the interface providing the basic close method.
struct closer {
  virtual void close() = 0;
};

template <int N>
struct bytebuffer : public reader, public writer {
  byte arr[N];
  int front;
  int back;
  bool err;

  const byte *bytes() { return arr; }

  int size() const { return back - front; }
  int max_size() const { return N; }
  bool error() const { return err; }

  void clear() {
    front = 0;
    back = 0;
    err = false;
  }

  bytebuffer() { clear(); }

  bytebuffer(const byte *s, int n) {
    clear();
    write(s, n);
  }

  int read(byte *s, int n) {
    if (err) {
      return 0;
    }

    int i = 0;

    while (i < n && front < back) {
      s[i++] = arr[front++];
    }

    if (i != n) {
      err = true;
    }

    realign();

    return i;
  }

  int readfrom(reader &r, int n) {
    byte block[64];
    int numblocks = n / 64;
    int bytes_read = 0;

    for (int i = 0; i < numblocks; i++) {
      r.read(block, 64);
      write(block, 64);
    }

    r.read(block, n);
    write(block, n);

    // TODO check for errors on this
    return n;
  }

  void realign() {
    if (front == 0) {
      return;
    }

    int n = back - front;

    for (int i = 0; i < n; i++) {
      arr[i] = arr[front + i];
    }

    front = 0;
    back = n;
  }

  int write(const byte *s, int n) {
    if (err) {
      return 0;
    }

    realign();

    int i = 0;

    while (i < n && back < max_size()) {
      arr[back++] = s[i++];
    }

    if (i != n) {
      err = true;
    }

    return i;
  }
};

// basic_encoder encodes basic types like bytes and numeric types. It is a
// building block for higher level encoders.
struct basic_encoder {
  writer &w;
  bool err;

  bool error() const { return err; }

  basic_encoder(writer &w) : w(w), err(false) {}

  void encode_bytes(const byte *s, int n) {
    if (err) {
      return;
    }

    if (w.write(s, n) != n) {
      err = true;
    }
  }

  void encode_uint(unsigned int x, int size) {
    if (err) {
      return;
    }

    // check if valid size
    if (size < 0 || size >= 8) {
      err = true;
      return;
    }

    byte s[size];

    // encode x in network byte order
    for (int i = size - 1; i >= 0; i--) {
      s[i] = (byte)x;
      x >>= 8;
    }

    encode_bytes(s, size);
  }
};

// basic_decoder decodes basic types like bytes and numeric types. It is a
// building block for higher level decoders.
struct basic_decoder {
  reader &r;
  bool err;

  bool error() const { return err; }

  basic_decoder(reader &r) : r(r), err(false) {}

  int decode_bytes(byte *s, int n) {
    if (err) {
      return 0;
    }

    int n2 = r.read(s, n);

    if (n2 != n) {
      err = true;
    }

    return n2;
  }

  unsigned int decode_uint(int size) {
    if (err) {
      return 0;
    }

    unsigned int x = 0;
    byte s[size];

    decode_bytes(s, size);

    if (err) {
      return 0;
    }

    for (int i = 0; i < size; i++) {
      x <<= 8;
      x |= (s[i] & 0xff);
    }

    return x;
  }
};

// TODO fix includes so order doesn't matter
#include "base64.h"
#include "crc.h"
#include "datagram.h"
#include "hex.h"
#include "sensorgram.h"

#endif
