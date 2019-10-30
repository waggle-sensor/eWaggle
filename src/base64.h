#ifndef __H_WAGGLE_BASE64__
#define __H_WAGGLE_BASE64__

const byte base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

struct base64_encoder : public writer, public closer {
  writer &w;
  byte buf[3];
  int nbuf;
  byte out[4];
  bool err;

  base64_encoder(writer &w) : w(w), nbuf(0), err(false) {}

  void close() {
    if (!err && nbuf > 0) {
      encode();
    }
  }

  int write(const byte *b, int n) {
    for (int i = 0; i < n; i++) {
      buf[nbuf++] = b[i];

      if (nbuf == 3) {
        encode();
      }
    }

    return n;
  }

  void encode() {
    if (nbuf == 1) {
      unsigned int x = (buf[0] << 16);
      out[0] = base64[(x >> 18) & 63];
      out[1] = base64[(x >> 12) & 63];
      out[2] = '=';
      out[3] = '=';
      w.write(out, 4);
    } else if (nbuf == 2) {
      unsigned int x = (buf[0] << 16) | (buf[1] << 8);
      out[0] = base64[(x >> 18) & 63];
      out[1] = base64[(x >> 12) & 63];
      out[2] = base64[(x >> 6) & 63];
      out[3] = '=';
      w.write(out, 4);
    } else if (nbuf == 3) {
      unsigned int x = (buf[0] << 16) | (buf[1] << 8) | buf[2];
      out[0] = base64[(x >> 18) & 63];
      out[1] = base64[(x >> 12) & 63];
      out[2] = base64[(x >> 6) & 63];
      out[3] = base64[x & 63];
      w.write(out, 4);
    } else {
      err = true;
    }

    nbuf = 0;
  }
};

struct base64_decoder : public reader {
  reader &r;
  byte buf[4];
  byte out[3];
  int remain;
  bool err;

  base64_decoder(reader &r) : r(r), remain(0), err(false) {}

  int read(byte *s, int n) {
    for (int i = 0; i < n; i++) {
      buf[remain++] = s[i];

      // this is wrong...= needs to signal decode time
      // accumulate 4 byte block for decoding
      if (remain < 4) {
        continue;
      }

      remain = 0;
    }

    return 0;
  }

  // TODO replace with more efficient search
  int base64_value(byte x) {
    if (x == '=') {
      return 0;
    }

    for (int i = 0; i < 64; i++) {
      if (x == base64[i]) {
        return i;
      }
    }

    err = true;
    return 0;
  }
};

#endif
