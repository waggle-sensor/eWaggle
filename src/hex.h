#ifndef __H_WAGGLE_HEX__
#define __H_WAGGLE_HEX__

const char base16[] = "0123456789abcdef";

struct hex_encoder : public writer {
  writer &w;

  hex_encoder(writer &w) : w(w) {}

  int write(const char *s, int n) {
    for (int i = 0; i < n; i++) {
      char b2[2];
      b2[0] = base16[(s[i] >> 4) & 0x0f];
      b2[1] = base16[s[i] & 0x0f];
      w.write(b2, 2);
    }

    return n;
  }
};

#endif
