#ifndef __H_WAGGLE__
#define __H_WAGGLE__

typedef unsigned char byte;

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

  void reset() {
    front = 0;
    back = 0;
    err = false;
  }

  bytebuffer() { reset(); }

  bytebuffer(const byte *s, int n) {
    reset();
    write(s, n);
  }

  void realign() {
    if (front == 0) {
      return;
    }

    int n = size();

    for (int i = 0; i < n; i++) {
      arr[i] = arr[front + i];
    }

    front = 0;
    back = n;
  }

  int read(byte *s, int n) {
    if (err) {
      return 0;
    }

    // error if no data to read
    if (size() == 0) {
      err = true;
      return 0;
    }

    int i = 0;

    while (i < n && front < back) {
      s[i++] = arr[front++];
    }

    return i;
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

    // error if buffer is too full
    if (i != n) {
      err = true;
    }

    return i;
  }

  int readfrom(reader &r, int n) {
    byte block[64];
    int ntotal = 0;

    while (n > 0) {
      int blocksize = 64;

      if (blocksize > n) {
        blocksize = n;
      }

      int nr = r.read(block, blocksize);
      int nw = write(block, nr);
      ntotal += nw;
      n -= blocksize;

      if (nw < blocksize) {
        break;
      }
    }

    return ntotal;
  }

  // int writeto(writer &w, int n) {
  //   if (n == -1) {
  //     n = size();
  //   }

  //   // could make more efficient...but doesn't matter for now.
  //   for (int i = 0; i < n; i++) {
  //     w.writebyte(r.readbyte());
  //   }

  //   // TODO check for errors on this
  //   return n;
  // }
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

#include "waggle/base64.h"
#include "waggle/crc.h"
#include "waggle/datagram.h"
#include "waggle/sensorgram.h"

#endif
