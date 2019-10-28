#ifndef __H_WAGGLE__
#define __H_WAGGLE__

int string_size(const char *s, int max_size) {
  for (int i = 0; i < max_size; i++) {
    if (s[i] == '\0') {
      return i;
    }
  }

  return max_size;
}

template <int N>
struct bytebuffer {
  char arr[N];
  int front;
  int back;
  bool err;

  const char *bytes() { return arr; }

  int size() const { return back - front; }
  int max_size() const { return N; }
  bool error() const { return err; }

  void clear() {
    front = 0;
    back = 0;
    err = false;
  }

  bytebuffer() { clear(); }

  int read(char *s, int n) {
    if (err) {
      return 0;
    }

    int i = 0;

    while (i < n && front < back) {
      s[i++] = arr[front++];
    }

    if (i != n) {
      err = true;
    }

    realign();

    return i;
  }

  void realign() {
    if (front == 0) {
      return;
    }

    int n = back - front;

    for (int i = 0; i < n; i++) {
      arr[i] = arr[front + i];
    }

    front = 0;
    back = n;
  }

  int write(const char *s, int n) {
    if (err) {
      return 0;
    }

    realign();

    int i = 0;

    while (i < n && back < max_size()) {
      arr[back++] = s[i++];
    }

    if (i != n) {
      err = true;
    }

    return i;
  }
};

struct bytereader {
  const char *buf;
  int pos;
  int cap;
  bool err;

  bool error() const { return err; }

  bytereader(const char *buf, int cap) : buf(buf), cap(cap) {
    pos = 0;
    err = false;
  }

  int read(char *b, int n) {
    if (err) {
      return 0;
    }

    int i = 0;

    while (i < n && pos < cap) {
      b[i++] = buf[pos++];
    }

    if (i != n) {
      err = true;
    }

    return i;
  }
};

// devnull is a "byte sink" used drop segments of data.
struct {
  int read(unsigned char *b, int n) { return 0; }
  int write(const unsigned char *b, int n) { return n; }
} devnull;

const unsigned char TYPE_NULL = 0x00;
const unsigned char TYPE_BYTES = 0x01;
const unsigned char TYPE_STRING = 0x02;

const unsigned char TYPE_INT8 = 0x03;
const unsigned char TYPE_UINT8 = 0x04;

const unsigned char TYPE_INT16 = 0x05;
const unsigned char TYPE_UINT16 = 0x06;

const unsigned char TYPE_INT24 = 0x07;
const unsigned char TYPE_UINT24 = 0x08;

const unsigned char TYPE_INT32 = 0x09;
const unsigned char TYPE_UINT32 = 0x0a;

const unsigned char TYPE_FLOAT16 = 0x0b;
const unsigned char TYPE_FLOAT32 = 0x0c;
const unsigned char TYPE_FLOAT64 = 0x0d;

// could even provide a buffer explicitly here instead of just int...?
template <int N>
struct sensorgram {
  unsigned long timestamp;
  unsigned int id;
  unsigned int inst;
  unsigned int sub_id;
  unsigned int source_id;
  unsigned int source_inst;
  bytebuffer<N> body;
};

template <class R, class W>
void copyn(R &r, W &w, int n) {
  unsigned char tmp[32];
  int copied = 0;

  while (copied < n) {
    int blocksize = n - copied;

    if (blocksize > sizeof(tmp)) {
      blocksize = sizeof(tmp);
    }

    if (r.read(tmp, blocksize) != blocksize) {
      return;
    }

    if (w.write(tmp, blocksize) != blocksize) {
      return;
    }

    copied += blocksize;
  }
}

template <class W>
void pack_bytes(W &w, const char *s, int n) {
  w.write(s, n);
}

template <class R>
void unpack_bytes(R &r, char *s, int n) {
  r.read(s, n);
}

template <class W>
void pack_uint(W &w, unsigned int x, int size) {
  char s[8];

  for (int i = size - 1; i >= 0; i--) {
    s[i] = (char)x;
    x >>= 8;
  }

  pack_bytes(w, s, size);
}

template <class R>
unsigned int unpack_uint(R &r, int size) {
  unsigned int x = 0;
  char s[8];

  unpack_bytes(r, s, size);

  for (int i = 0; i < size; i++) {
    x <<= 8;
    x |= (s[i] & 0xff);
  }

  return x;
}

// template as array type handler?
template <class W>
void pack_string_val(W &w, const char *s) {
  int size = string_size(s, 1024);
  pack_uint(w, TYPE_STRING, 1);
  pack_uint(w, size, 2);
  w.write(s, size);
}

template <class R>
void unpack_string_val(R &r, char *s) {
  unsigned char b[1];

  r.read(b, 1);

  if (b[0] != TYPE_STRING) {
    return;
  }

  while (r.read(b, 1) == 1 && b[0] != '\0') {
    *s++ = b[0];
  }

  *s = '\0';
}

template <class W>
void pack_float32(W &w, float x) {
  // WARNING Possibly unsafe and unreliable across platforms. Check this
  // carefully!
  unsigned char *b = (unsigned char *)&x;
  pack_bytes(w, b, 4);
}

template <class W>
void pack_float64(W &w, double x) {
  // WARNING Possibly unsafe and unreliable across platforms. Check this
  // carefully!
  unsigned char *b = (unsigned char *)&x;
  pack_bytes(w, b, 8);
}

template <class R>
float unpack_float32(R &r) {
  unsigned char b[4];
  unpack_bytes(r, b, 4);
  return *(float *)b;
}

template <class R>
double unpack_float64(R &r) {
  unsigned char b[8];
  unpack_bytes(r, b, 8);
  return *(double *)b;
}

template <class W, class SG>
void pack_sensorgram(W &w, SG &sg) {
  pack_uint(w, sg.body.size(), 2);
  pack_uint(w, sg.timestamp, 4);
  pack_uint(w, sg.id, 2);
  pack_uint(w, sg.inst, 1);
  pack_uint(w, sg.sub_id, 1);
  pack_uint(w, sg.source_id, 2);
  pack_uint(w, sg.source_inst, 1);
  pack_bytes(w, sg.body.bytes(), sg.body.size());
}

template <class R, class SG>
bool unpack_sensorgram(R &r, SG &sg) {
  int len = unpack_uint(r, 2);
  sg.timestamp = unpack_uint(r, 4);
  sg.id = unpack_uint(r, 2);
  sg.inst = unpack_uint(r, 1);
  sg.sub_id = unpack_uint(r, 1);
  sg.source_id = unpack_uint(r, 2);
  sg.source_inst = unpack_uint(r, 1);

  sg.body.clear();
  copyn(r, sg.body, len);

  return !r.error();
}

template <class W>
void pack_uint_val(W &w, unsigned long x) {
  if (x <= 0xff) {
    pack_uint(w, TYPE_UINT8, 1);
    pack_uint(w, x, 1);
    return;
  }

  if (x <= 0xffff) {
    pack_uint(w, TYPE_UINT16, 1);
    pack_uint(w, x, 2);
    return;
  }

  if (x <= 0xffffff) {
    pack_uint(w, TYPE_UINT24, 1);
    pack_uint(w, x, 3);
    return;
  }

  pack_uint(w, TYPE_UINT32, 1);
  pack_uint(w, x, 4);
}

template <class W>
void pack_float_val(W &w, float x) {
  pack_uint(w, TYPE_FLOAT32, 1);
  pack_float32(w, x);
}

template <class W>
void pack_double_val(W &w, double x) {
  pack_uint(w, TYPE_FLOAT64, 1);
  pack_float64(w, x);
}

template <class R>
unsigned long unpack_uint_val(R &r) {
  unsigned int type = unpack_uint(r, 1);

  switch (type) {
    case TYPE_UINT8:
      return unpack_uint(r, 1);
    case TYPE_UINT16:
      return unpack_uint(r, 2);
    case TYPE_UINT24:
      return unpack_uint(r, 3);
    case TYPE_UINT32:
      return unpack_uint(r, 4);
  }

  return 0;
}

template <class R>
float unpack_float_val(R &r) {
  if (unpack_uint(r, 1) != TYPE_FLOAT32) {
    return 0;
  }

  return unpack_float32(r);
}

template <class R>
float unpack_double_val(R &r) {
  if (unpack_uint(r, 1) != TYPE_FLOAT64) {
    return 0;
  }

  return unpack_float64(r);
}

const unsigned char crc8_table[256] = {
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

unsigned char calc_crc8(const unsigned char *b, int n) {
  unsigned char crc = 0;

  for (int i = 0; i < n; i++) {
    crc = crc8_table[crc ^ b[i]];
  }

  return crc;
}

template <int N>
struct datagram {
  unsigned int protocol_version;
  unsigned int timestamp;
  unsigned int packet_seq;
  unsigned int packet_type;
  unsigned int plugin_id;
  unsigned int plugin_major_version;
  unsigned int plugin_minor_version;
  unsigned int plugin_patch_version;
  unsigned int plugin_instance;
  unsigned int plugin_run_id;
  bytebuffer<N> body;
};

// Now, we can use different functions for actually sending / recving
// datagrams.

template <class W, class DG>
void pack_datagram(W &w, DG &dg) {
  pack_uint(w, dg.body.size(), 3);         // [Length (3B)]
  pack_uint(w, dg.ProtocolVersion, 1);     // [Protocol_version (1B)]
  pack_uint(w, dg.Timestamp, 4);           // [time (4B)]
  pack_uint(w, dg.PacketSeq, 2);           // [Packet_Seq (2B)]
  pack_uint(w, dg.PacketType, 1);          // [Packet_type (1B)]
  pack_uint(w, dg.PluginID, 2);            // [Plugin ID (2B)]
  pack_uint(w, dg.PluginMajorVersion, 1);  // [Plugin Maj Ver (1B)]
  pack_uint(w, dg.PluginMinorVersion, 1);  // [Plugin Min Ver (1B)]
  pack_uint(w, dg.PluginPatchVersion, 1);  // [Plugin Build Ver (1B)]
  pack_uint(w, dg.PluginInstance, 1);      // [Plugin Instance (1B)]
  pack_uint(w, dg.PluginRunID, 2);         // [Plugin Run ID (2B)]
  pack_bytes(w, dg.body.bytes(), dg.body.size());

  // framing footer
  pack_uint(w, calc_crc8(dg.body.bytes(), dg.body.size()), 1);  // [CRC (1B)]
  // TODO make crc16 instead of 8. and computed against header + body...
  // length should be length all the way until the end...
}

int find_byte(unsigned char x, const unsigned char *b, int n) {
  for (int i = 0; i < n; i++) {
    if (b[i] == x) {
      return i;
    }
  }

  return -1;
}

// need to have a way of indicating whether a datagram was actually unpacked or
// not.
template <class B, class DG>
bool unpack_datagram(B &buf, DG &dg) {
  bytereader r(buf.bytes(), buf.size());

  int len = unpack_uint(r, 3);                  // [Length (3B)]
  dg.protocol_version = unpack_uint(r, 1);      // [Protocol_version (1B)]
  dg.timestamp = unpack_uint(r, 4);             // [time (4B)]
  dg.packet_seq = unpack_uint(r, 2);            // [Packet_Seq (2B)]
  dg.packet_type = unpack_uint(r, 1);           // [Packet_type (1B)]
  dg.plugin_id = unpack_uint(r, 2);             // [Plugin ID (2B)]
  dg.plugin_major_version = unpack_uint(r, 1);  // [Plugin Maj Ver (1B)]
  dg.plugin_minor_version = unpack_uint(r, 1);  // [Plugin Min Ver (1B)]
  dg.plugin_patch_version = unpack_uint(r, 1);  // [Plugin Build Ver (1B)]
  dg.plugin_instance = unpack_uint(r, 1);       // [Plugin Instance (1B)]
  dg.plugin_run_id = unpack_uint(r, 2);         // [Plugin Run ID (2B)]

  // Is this consistent with Pack??
  dg.body.clear();
  copyn(r, dg.body, len);

  if (r.error()) {
    return false;
  }

  unsigned char recv_crc = unpack_uint(r, 1);
  unsigned char calc_crc = calc_crc8(dg.body.bytes(), dg.body.size());

  if (recv_crc != calc_crc) {
    return false;
  }

  return !r.error();
}

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

#endif
