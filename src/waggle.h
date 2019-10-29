#ifndef __H_WAGGLE__
#define __H_WAGGLE__

// reader is the interface providing the basic read method.
struct reader {
  virtual int read(char *s, int n) = 0;
};

// writer is the interface providing the basic write method.
struct writer {
  virtual int write(const char *s, int n) = 0;
};

// closer is the interface providing the basic close method.
struct closer {
  virtual void close() = 0;
};

int string_size(const char *s, int max_size) {
  for (int i = 0; i < max_size; i++) {
    if (s[i] == '\0') {
      return i;
    }
  }

  return max_size;
}

template <int N>
struct bytebuffer : public reader, public writer {
  char arr[N];
  int front;
  int back;
  bool err;

  const char *bytes() { return arr; }

  int size() const { return back - front; }
  int max_size() const { return N; }
  bool error() const { return err; }

  void clear() {
    front = 0;
    back = 0;
    err = false;
  }

  bytebuffer() { clear(); }

  int read(char *s, int n) {
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
    char block[64];
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

  int write(const char *s, int n) {
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

struct bytereader : public reader {
  const char *buf;
  int pos;
  int cap;
  bool err;

  bool error() const { return err; }

  bytereader(const char *buf, int cap) : buf(buf), cap(cap) {
    pos = 0;
    err = false;
  }

  int read(char *b, int n) {
    if (err) {
      return 0;
    }

    int i = 0;

    while (i < n && pos < cap) {
      b[i++] = buf[pos++];
    }

    if (i != n) {
      err = true;
    }

    return i;
  }
};

const char TYPE_NULL = 0x00;
const char TYPE_BYTES = 0x01;
const char TYPE_STRING = 0x02;

const char TYPE_INT8 = 0x03;
const char TYPE_UINT8 = 0x04;

const char TYPE_INT16 = 0x05;
const char TYPE_UINT16 = 0x06;

const char TYPE_INT24 = 0x07;
const char TYPE_UINT24 = 0x08;

const char TYPE_INT32 = 0x09;
const char TYPE_UINT32 = 0x0a;

const char TYPE_FLOAT16 = 0x0b;
const char TYPE_FLOAT32 = 0x0c;
const char TYPE_FLOAT64 = 0x0d;

// basic_encoder encodes basic types like bytes and numeric types. It is a
// building block for higher level encoders.
struct basic_encoder {
  writer &w;
  bool err;

  basic_encoder(writer &w) : w(w), err(false) {}

  void encode_bytes(const char *s, int n) {
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

    char s[8];

    // encode x in network byte order
    for (int i = size - 1; i >= 0; i--) {
      s[i] = (char)x;
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

  basic_decoder(reader &r) : r(r), err(false) {}

  void decode_bytes(char *s, int n) {
    if (err) {
      return;
    }

    if (r.read(s, n) != n) {
      err = true;
    }
  }

  unsigned int decode_uint(int size) {
    if (err) {
      return 0;
    }

    // check if valid size
    if (size < 0 || size >= 8) {
      err = true;
      return 0;
    }

    unsigned int x = 0;
    char s[8];

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
#include "sensorgram.h"

#endif
