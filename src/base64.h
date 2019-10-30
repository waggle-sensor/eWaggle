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

// struct base64_decoder : public reader {
//   reader &r;
//   byte buf[4];
//   int nbuf;
//   byte out[3];
//   bool err;

//   base64_decoder(reader &r) : r(r), nbuf(0), err(false) {}

//   int read(byte *s, int n) {
//     // need to update state to read n bytes

//     // ah... right this isn't the implementation we're expecting...

//     for (int i = 0; i < n; i++) {
//       if (s[i] == '=') {
//         decode();
//         continue;
//       }

//       buf[nbuf++] = s[i];

//       if (nbuf == 4) {
//         decode();
//       }
//     }

//     return 0;
//   }

//   void decode() {
//     unsigned int x = 0;

//     if (nbuf == 1) {
//       x = valueof(buf[0]) << 18;
//     } else if (nbuf == 2) {
//       x = (valueof(buf[0]) << 18) | (valueof(buf[1]) << 12);
//     } else if (nbuf == 3) {
//       x = (valueof(buf[0]) << 18) | (valueof(buf[1]) << 12) |
//           (valueof(buf[2]) << 6);
//     } else if (nbuf == 4) {
//       x = (valueof(buf[0]) << 18) | (valueof(buf[1]) << 12) |
//           (valueof(buf[2]) << 6) | valueof(buf[3]);
//     } else {
//       err = true;
//       return;
//     }

//     out[0] = (x >> 16) & 0xff;
//     out[1] = (x >> 8) & 0xff;
//     out[2] = x & 0xff;
//     nbuf = 0;
//   }

//   // TODO replace with more efficient search
//   int valueof(byte x) {
//     for (int i = 0; i < 64; i++) {
//       if (x == base64[i]) {
//         return i;
//       }
//     }

//     err = true;
//     return 0;
//   }
// };

#endif
