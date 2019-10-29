#ifndef __H_WAGGLE_HEX__
#define __H_WAGGLE_HEX__

const byte base16[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

struct hex_encoder : public writer {
  writer &w;

  hex_encoder(writer &w) : w(w) {}

  int write(const byte *s, int n) {
    for (int i = 0; i < n; i++) {
      byte b2[2];
      b2[0] = base16[(s[i] >> 4) & 0x0f];
      b2[1] = base16[s[i] & 0x0f];
      w.write(b2, 2);
    }

    return n;
  }
};

#endif
