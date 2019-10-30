#ifndef __H_WAGGLE_BASE64__
#define __H_WAGGLE_BASE64__

const byte base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

byte b64value(byte x) {
  for (int i = 0; i < 64; i++) {
    if (x == base64[i]) {
      return i;
    }
  }

  return 0;

  if ('A' <= x && x <= 'Z') {
    return x - 'A';
  }

  if ('a' <= x && x <= 'z') {
    return (x - 'a') + ('Z' - 'A');
  }

  if ('0' <= x && x <= '9') {
    return (x - '0') + (('z' - 'a') + ('Z' - 'A'));
  }

  if (x == '+') {
    return 62;
  }

  if (x == '/') {
    return 63;
  }

  return 0;
}

int b64encode(const byte *buf, int n, byte *out) {
  int pos = 0;
  int nblocks = n / 3;

  // encode all full blocks
  for (int i = 0; i < nblocks; i++) {
    unsigned int x = (buf[0] << 16) | (buf[1] << 8) | buf[2];
    buf += 3;
    out[pos++] = base64[(x >> 18) & 63];
    out[pos++] = base64[(x >> 12) & 63];
    out[pos++] = base64[(x >> 6) & 63];
    out[pos++] = base64[x & 63];
  }

  int remain = n - 3 * nblocks;

  // encode remaining bytes
  if (remain == 1) {
    unsigned int x = (buf[0] << 16);
    out[pos++] = base64[(x >> 18) & 63];
    out[pos++] = base64[(x >> 12) & 63];
    out[pos++] = '=';
    out[pos++] = '=';
  } else if (remain == 2) {
    unsigned int x = (buf[0] << 16) | (buf[1] << 8);
    out[pos++] = base64[(x >> 18) & 63];
    out[pos++] = base64[(x >> 12) & 63];
    out[pos++] = base64[(x >> 6) & 63];
    out[pos++] = '=';
  }

  out[pos] = 0;
  return pos;
}

void b64decode(const byte *buf, int n, writer &w) {
  byte out[3];

  // trim padding
  while ((buf[n - 1] == 0) || (buf[n - 1] == '=')) {
    n--;
  }

  int nblocks = n / 4;

  // decode full blocks
  for (int i = 0; i < nblocks; i++) {
    unsigned int x = (b64value(buf[0]) << 18) | (b64value(buf[1]) << 12) |
                     (b64value(buf[2]) << 6) | b64value(buf[3]);
    buf += 4;
    out[0] = (x >> 16) & 0xff;
    out[1] = (x >> 8) & 0xff;
    out[2] = x & 0xff;
    w.write(out, 3);
  }

  //
  //   1        2       3
  // 000000001111111122222222
  // 000000111111222222333333
  //   1     2      3     4
  //

  int remain = n - 4 * nblocks;

  // decode remaining bytes
  if (remain == 1) {
    unsigned int x = (b64value(buf[0]) << 18);
    out[0] = (x >> 16) & 0xff;
    w.write(out, 1);
  } else if (remain == 2) {
    unsigned int x = (b64value(buf[0]) << 18) | (b64value(buf[1]) << 12);
    out[0] = (x >> 16) & 0xff;
    // out[1] = (x >> 8) & 0xff;
    w.write(out, 1);
  } else if (remain == 3) {
    unsigned int x = (b64value(buf[0]) << 18) | (b64value(buf[1]) << 12) |
                     (b64value(buf[2]) << 6);
    out[0] = (x >> 16) & 0xff;
    out[1] = (x >> 8) & 0xff;
    // out[2] = x & 0xff;
    w.write(out, 2);
  }
}

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
    } else if (nbuf == 2) {
      unsigned int x = (buf[0] << 16) | (buf[1] << 8);
      out[0] = base64[(x >> 18) & 63];
      out[1] = base64[(x >> 12) & 63];
      out[2] = base64[(x >> 6) & 63];
      out[3] = '=';
    } else if (nbuf == 3) {
      unsigned int x = (buf[0] << 16) | (buf[1] << 8) | buf[2];
      out[0] = base64[(x >> 18) & 63];
      out[1] = base64[(x >> 12) & 63];
      out[2] = base64[(x >> 6) & 63];
      out[3] = base64[x & 63];
    } else {
      err = true;
      return;
    }

    w.write(out, 4);
    nbuf = 0;
  }
};

struct base64_decoder : public reader {
  reader &r;
  byte buf[4];
  bytebuffer<3> out;
  bool err;

  base64_decoder(reader &r) : r(r), err(false) {}

  int read(byte *s, int n) {
    for (int i = 0; i < n; i++) {
      s[i] = decodebyte();
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
      unsigned int x = (b64value(buf[0]) << 18);
      out.writebyte((x >> 16) & 0xff);
    } else if (n == 2) {
      unsigned int x = (b64value(buf[0]) << 18) | (b64value(buf[1]) << 12);
      out.writebyte((x >> 16) & 0xff);
    } else if (n == 3) {
      unsigned int x = (b64value(buf[0]) << 18) | (b64value(buf[1]) << 12) |
                       (b64value(buf[2]) << 6);
      out.writebyte((x >> 16) & 0xff);
      out.writebyte((x >> 8) & 0xff);
    } else if (n == 4) {
      unsigned int x = (b64value(buf[0]) << 18) | (b64value(buf[1]) << 12) |
                       (b64value(buf[2]) << 6) | b64value(buf[3]);
      out.writebyte((x >> 16) & 0xff);
      out.writebyte((x >> 8) & 0xff);
      out.writebyte(x & 0xff);
    }
  }
};

#endif
