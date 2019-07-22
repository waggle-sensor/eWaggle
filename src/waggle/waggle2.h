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
template <int cap>
struct Sensorgram {
  unsigned long Timestamp;
  unsigned int ID;
  unsigned int Inst;
  unsigned int SubID;
  unsigned int SourceID;
  unsigned int SourceInst;
  Buffer<cap> Body;
};

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

template <class W, class SG>
void PackSensorgram(W &w, SG &sg) {
  PackUint16(w, sg.Body.Len() + SensorgramHeaderLength);
  PackUint32(w, sg.Timestamp);
  PackUint16(w, sg.ID);
  PackUint8(w, sg.Inst);
  PackUint8(w, sg.SubID);
  PackUint16(w, sg.SourceID);
  PackUint8(w, sg.SourceInst);
  PackBytes(w, sg.Body.Bytes(), sg.Body.Len());
}

template <class R, class SG>
bool UnpackSensorgram(R &r, SG &sg) {
  int len = UnpackUint16(r);
  sg.Timestamp = UnpackUint32(r);
  sg.ID = UnpackUint16(r);
  sg.Inst = UnpackUint8(r);
  sg.SubID = UnpackUint8(r);
  sg.SourceID = UnpackUint16(r);
  sg.SourceInst = UnpackUint8(r);

  sg.Body.Reset();
  Copy(r, sg.Body, len - SensorgramHeaderLength);

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

const unsigned char crcTable[256] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 0xc2, 0x9c, 0x7e, 0x20,
    0xa3, 0xfd, 0x1f, 0x41, 0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
    0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc, 0x23, 0x7d, 0x9f, 0xc1,
    0x42, 0x1c, 0xfe, 0xa0, 0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 0x7c, 0x22, 0xc0, 0x9e,
    0x1d, 0x43, 0xa1, 0xff, 0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
    0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07, 0xdb, 0x85, 0x67, 0x39,
    0xba, 0xe4, 0x06, 0x58, 0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 0xa7, 0xf9, 0x1b, 0x45,
    0xc6, 0x98, 0x7a, 0x24, 0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
    0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9, 0x8c, 0xd2, 0x30, 0x6e,
    0xed, 0xb3, 0x51, 0x0f, 0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92, 0xd3, 0x8d, 0x6f, 0x31,
    0xb2, 0xec, 0x0e, 0x50, 0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
    0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee, 0x32, 0x6c, 0x8e, 0xd0,
    0x53, 0x0d, 0xef, 0xb1, 0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49, 0x08, 0x56, 0xb4, 0xea,
    0x69, 0x37, 0xd5, 0x8b, 0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
    0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16, 0xe9, 0xb7, 0x55, 0x0b,
    0x88, 0xd6, 0x34, 0x6a, 0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 0xb6, 0xe8, 0x0a, 0x54,
    0xd7, 0x89, 0x6b, 0x35,
};

unsigned char crc8(const unsigned char *data, int size, unsigned char crc = 0) {
  for (int i = 0; i < size; i++) {
    crc = crcTable[crc ^ data[i]];
  }

  return crc;
}

template <int cap>
struct Datagram {
  unsigned int ProtocolVersion;
  unsigned int Timestamp;
  unsigned int PacketSeq;
  unsigned int PacketType;
  unsigned int PluginID;
  unsigned int PluginMajorVersion;
  unsigned int PluginMinorVersion;
  unsigned int PluginPatchVersion;
  unsigned int PluginInstance;
  unsigned int PluginRunID;
  Buffer<cap> Body;
};

// Now, we can use different functions for actually sending / recving
// datagrams.

const int DatagramHeaderLength = 1+3+1+4+2+1+2+1+1+1+2;
const int DatagramFooterLength = 1+1;

template <class W, class DG>
void PackDatagram(W &w, DG &dg) {
  PackUint8(w, 0xaa); // [Start_Byte (1B)]
  PackUint24(w, dg.Body.Len() + DatagramHeaderLength + DatagramFooterLength); // [Length (3B)]
  PackUint8(w, dg.ProtocolVersion); // [Protocol_version (1B)]
  PackUint32(w, dg.Timestamp); // [time (4B)]
  PackUint16(w, dg.PacketSeq); // [Packet_Seq (2B)]
  PackUint8(w, dg.PacketType); // [Packet_type (1B)]
  PackUint16(w, dg.PluginID); // [Plugin ID (2B)]
  PackUint8(w, dg.PluginMajorVersion); // [Plugin Maj Ver (1B)]
  PackUint8(w, dg.PluginMinorVersion); // [Plugin Min Ver (1B)]
  PackUint8(w, dg.PluginPatchVersion); // [Plugin Build Ver (1B)]
  PackUint8(w, dg.PluginInstance); // [Plugin Instance (1B)]
  PackUint16(w, dg.PluginRunID); // [Plugin Run ID (2B)]
  PackBytes(w, dg.Body.Bytes(), dg.Body.Len());
  PackUint8(w, crc8(dg.Body.Bytes(), dg.Body.Len())); // [CRC (1B)]
  PackUint8(w, 0x55); // [End_Byte (1B)]
}
