#ifndef __H_WAGGLE_BASE64__
#define __H_WAGGLE_BASE64__

#include <stdint.h>
#include "waggle/bytebuffer.h"

const byte base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

struct base64_encoder final : public writer, public closer {
  writer &w;
  byte buf[3];
  int nbuf;
  byte out[4];
  bool err;

  base64_encoder(writer &w) : w(w), nbuf(0), err(false) {}

  void close() {
    if (err) {
      return;
    }

    if (nbuf > 0) {
      encode();
    }
  }

  int write(const byte *b, int n) {
    if (err) {
      return 0;
    }

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
      uint32_t x = ((uint32_t)buf[0] << 16);
      out[0] = base64[(x >> 18) & 63];
      out[1] = base64[(x >> 12) & 63];
      out[2] = '=';
      out[3] = '=';
    } else if (nbuf == 2) {
      uint32_t x = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8);
      out[0] = base64[(x >> 18) & 63];
      out[1] = base64[(x >> 12) & 63];
      out[2] = base64[(x >> 6) & 63];
      out[3] = '=';
    } else if (nbuf == 3) {
      uint32_t x =
          ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | (uint32_t)buf[2];
      out[0] = base64[(x >> 18) & 63];
      out[1] = base64[(x >> 12) & 63];
      out[2] = base64[(x >> 6) & 63];
      out[3] = base64[x & 63];
    } else {
      err = true;
      return;
    }

    if (w.write(out, 4) != 4) {
      err = true;
    }

    nbuf = 0;
  }
};

struct base64_decoder final : public reader {
  reader &r;
  byte buf[4];
  bytebuffer<3> out;
  bool err;

  base64_decoder(reader &r) : r(r), err(false) {}

  int read(byte *s, int n) {
    for (int i = 0; i < n; i++) {
      s[i] = decodebyte();

      if (err) {
        return i;
      }
    }

    return n;
  }

  byte decodebyte() {
    if (out.size() == 0) {
      readbuffer();
    }

    if (err) {
      return 0;
    }

    return out.readbyte();
  }

  void readbuffer() {
    int n = r.read(buf, 4);

    // trim padding
    while (n > 0 && buf[n - 1] == '=') {
      n--;
    }

    if (n == 0) {
      err = true;
      return;
    }

    if (n == 1) {
      uint32_t x = (b64value(buf[0]) << 18);
      out.writebyte((x >> 16) & 0xff);
    } else if (n == 2) {
      uint32_t x = (b64value(buf[0]) << 18) | (b64value(buf[1]) << 12);
      out.writebyte((x >> 16) & 0xff);
    } else if (n == 3) {
      uint32_t x = (b64value(buf[0]) << 18) | (b64value(buf[1]) << 12) |
                   (b64value(buf[2]) << 6);
      out.writebyte((x >> 16) & 0xff);
      out.writebyte((x >> 8) & 0xff);
    } else if (n == 4) {
      uint32_t x = (b64value(buf[0]) << 18) | (b64value(buf[1]) << 12) |
                   (b64value(buf[2]) << 6) | b64value(buf[3]);
      out.writebyte((x >> 16) & 0xff);
      out.writebyte((x >> 8) & 0xff);
      out.writebyte(x & 0xff);
    }

    if (out.err) {
      err = true;
    }
  }

  uint32_t b64value(uint32_t x) {
    if ('A' <= x && x <= 'Z') {
      return x - 'A';
    }

    if ('a' <= x && x <= 'z') {
      return (x - 'a') + 26;
    }

    if ('0' <= x && x <= '9') {
      return (x - '0') + 52;
    }

    if (x == '+') {
      return 62;
    }

    if (x == '/') {
      return 63;
    }

    err = true;
    return 0;
  }
};

#endif
