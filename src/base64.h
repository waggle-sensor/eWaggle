const char base64[] =
    ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "abcdefghijklmnopqrstuvwxyz"
     "0123456789+/");

template <typename writerT>
struct base64encoder {
  writerT &w;

  bool closed;
  int remain;
  char b3[3];

  base64encoder(writerT &w) : w(w) {
    remain = 0;
    closed = false;
  }

  ~base64encoder() { close(); }

  void close() {
    if (closed) {
      return;
    }

    closed = true;

    char b4[4];

    if (remain == 1) {
      int x = (b3[0] << 16);
      b4[0] = base64[(x >> 18) & 63];
      b4[1] = base64[(x >> 12) & 63];
      w.write(b4, 2);
    } else if (remain == 2) {
      int x = (b3[0] << 16) | (b3[1] << 8);
      b4[0] = base64[(x >> 18) & 63];
      b4[1] = base64[(x >> 12) & 63];
      b4[2] = base64[(x >> 6) & 63];
      w.write(b4, 3);
    }
  }

  void write(const char *b, int n) {
    if (closed) {
      return;
    }

    for (int i = 0; i < n; i++) {
      b3[remain++] = b[i];

      if (remain == 3) {
        remain = 0;

        char b4[4];
        int x = (b3[0] << 16) | (b3[1] << 8) | b3[2];
        b4[0] = base64[(x >> 18) & 63];
        b4[1] = base64[(x >> 12) & 63];
        b4[2] = base64[(x >> 6) & 63];
        b4[3] = base64[x & 63];
        w.write(b4, 4);
      }
    }
  }
};
