#ifndef __H_WAGGLE_BYTEBUFFER__
#define __H_WAGGLE_BYTEBUFFER__

template <int N>
struct bytebuffer final : public reader, public writer {
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

#endif
