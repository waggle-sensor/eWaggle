#ifndef __H_WAGGLE_BASIC__
#define __H_WAGGLE_BASIC__

// basic_encoder encodes basic types like bytes and numeric types. It is a
// building block for higher level encoders.
struct basic_encoder final {
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
struct basic_decoder final {
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

#endif
