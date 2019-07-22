template <class R, class W>
void Copy(R &r, W &w, int n) {
  unsigned char tmp[32];
  int copied = 0;

  while (copied < n) {
    int blocksize = n - copied;

    if (blocksize > sizeof(tmp)) {
      blocksize = sizeof(tmp);
    }

    if (r.Read(tmp, blocksize) != blocksize) {
      return;
    }

    if (w.Write(tmp, blocksize) != blocksize) {
      return;
    }

    copied += blocksize;
  }
}

template <class W>
void PackBytes(W &w, const unsigned char *b, int n) {
  // for bytes values, can include the len...that's the simplest thing...?
  w.Write(b, n);
}

int stringLen(const char *s) {
  int n = 0;

  while (s[n] != '\0') {
    n++;
  }

  return n;
}

template <class W>
void PackString(W &w, const char *s) {
  // we include the null terminator from string.
  w.Write((const unsigned char *)s, stringLen(s) + 1);
}

template <class W>
void PackUint8(W &w, unsigned int x) {
  unsigned char b[] = {x};
  PackBytes(w, b, 1);
}

template <class W>
void PackUint16(W &w, unsigned int x) {
  unsigned char b[] = {x >> 8, x};
  PackBytes(w, b, 2);
}

template <class W>
void PackUint24(W &w, unsigned int x) {
  unsigned char b[] = {x >> 16, x >> 8, x};
  PackBytes(w, b, 3);
}

template <class W>
void PackUint32(W &w, unsigned int x) {
  unsigned char b[] = {x >> 24, x >> 16, x >> 8, x};
  PackBytes(w, b, 4);
}

template <class W>
void PackFloat32(W &w, float x) {
  // WARNING Possibly unsafe and unreliable across platforms. Check this
  // carefully!
  unsigned char *b = (unsigned char *)&x;
  PackBytes(w, b, 4);
}

template <class W>
void PackFloat64(W &w, double x) {
  // WARNING Possibly unsafe and unreliable across platforms. Check this
  // carefully!
  unsigned char *b = (unsigned char *)&x;
  PackBytes(w, b, 8);
}

template <class R>
void UnpackBytes(R &r, unsigned char *b, int n) {
  r.Read(b, n);
}

template <class R>
void UnpackString(R &r, char *s) {
  // warning unsafe
  unsigned char b[1];

  while (r.Read(b, 1) == 1 && b[0] != '\0') {
    *s++ = b[0];
  }

  *s = '\0';
}

template <class R>
unsigned int UnpackUint8(R &r) {
  unsigned char b[1];
  UnpackBytes(r, b, 1);
  return b[0];
}

template <class R>
unsigned int UnpackUint16(R &r) {
  unsigned char b[2];
  UnpackBytes(r, b, 2);
  return (b[0] << 8) | b[1];
}

template <class R>
unsigned int UnpackUint24(R &r) {
  unsigned char b[3];
  UnpackBytes(r, b, 3);
  return (b[0] << 16) | (b[1] << 8) | b[2];
}

template <class R>
unsigned int UnpackUint32(R &r) {
  unsigned char b[4];
  UnpackBytes(r, b, 4);
  return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

template <class R>
float UnpackFloat32(R &r) {
  unsigned char b[4];
  UnpackBytes(r, b, 4);
  return *(float *)b;
}

template <class R>
double UnpackFloat64(R &r) {
  unsigned char b[8];
  UnpackBytes(r, b, 8);
  return *(double *)b;
}

const int SensorgramHeaderLength = 4 + 2 + 1 + 1 + 2 + 2;

template <class W, class S>
void PackSensorgram(W &w, S &s) {
  PackUint16(w, s.Body.Len() + SensorgramHeaderLength);
  PackUint32(w, s.Timestamp);
  PackUint16(w, s.ID);
  PackUint8(w, s.Inst);
  PackUint8(w, s.SubID);
  PackUint16(w, s.SourceID);
  PackUint8(w, s.SourceInst);
  PackBytes(w, s.Body.Bytes(), s.Body.Len());
}

template <class R, class S>
bool UnpackSensorgram(R &r, S &s) {
  int len = UnpackUint16(r);
  s.Timestamp = UnpackUint32(r);
  s.ID = UnpackUint16(r);
  s.Inst = UnpackUint8(r);
  s.SubID = UnpackUint8(r);
  s.SourceID = UnpackUint16(r);
  s.SourceInst = UnpackUint8(r);

  s.Body.Reset();
  Copy(r, s.Body, len - SensorgramHeaderLength);

  return !r.Err();
}

template <class W>
void PackUintVal(W &w, unsigned long x) {
  if (x <= 0xff) {
    PackUint8(w, 0x10);
    PackUint8(w, x);
    return;
  }

  if (x <= 0xffff) {
    PackUint8(w, 0x11);
    PackUint16(w, x);
    return;
  }

  if (x <= 0xffffff) {
    PackUint8(w, 0x12);
    PackUint24(w, x);
    return;
  }

  PackUint8(w, 0x13);
  PackUint32(w, x);
}

template <class R>
unsigned long UnpackUintVal(R &r) {
  unsigned int type = UnpackUint8(r);

  switch (type) {
    case 0x10:
      return UnpackUint8(r);
    case 0x11:
      return UnpackUint16(r);
    case 0x12:
      return UnpackUint24(r);
    case 0x13:
      return UnpackUint32(r);
  }

  return 0;
}

template <int cap>
struct Buffer {
  unsigned char buf[cap];
  int pos;
  int len;
  bool err;  // hack for now... don't have a clean way to handle results

  const unsigned char *Bytes() {
    if (pos > 0) {
      int i = 0;

      while (pos < len) {
        buf[i++] = buf[pos++];
      }

      pos = 0;
      len = i;
    }

    return buf;
  }

  int Len() const { return len - pos; }
  int Cap() const { return cap; }
  bool Err() const { return err; }

  void Reset() {
    pos = 0;
    len = 0;
    err = false;
  }

  Buffer() { Reset(); }

  int Read(unsigned char *b, int n) {
    if (err) {
      return 0;
    }

    int i = 0;

    while (i < n && pos < len) {
      b[i++] = buf[pos++];
    }

    // reset buffer here..
    if (pos == len) {
    }

    if (i != n) {
      err = true;
    }

    return i;
  }

  int Write(const unsigned char *b, int n) {
    if (err) {
      return 0;
    }

    int i = 0;

    while (i < n && len < cap) {
      buf[len++] = b[i++];
    }

    if (i != n) {
      err = true;
    }

    return i;
  }
};

// could even provide a buffer explicitly here instead of just int...?
template <int N>
struct Sensorgram {
  unsigned long Timestamp;
  unsigned int ID;
  unsigned int Inst;
  unsigned int SubID;
  unsigned int SourceID;
  unsigned int SourceInst;
  Buffer<N> Body;
};
