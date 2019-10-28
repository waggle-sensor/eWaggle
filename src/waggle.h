#ifndef __H_WAGGLE__
#define __H_WAGGLE__

#include "base64.h"
#include "crc.h"

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
  int read(char *s, int n) { return 0; }
  int write(const char *s, int n) { return n; }
} devnull;

const char TYPE_NULL = 0x00;
const char TYPE_BYTES = 0x01;
const char TYPE_STRING = 0x02;

const char TYPE_INT8 = 0x03;
const char TYPE_UINT8 = 0x04;

const char TYPE_INT16 = 0x05;
const char TYPE_UINT16 = 0x06;

const char TYPE_INT24 = 0x07;
const char TYPE_UINT24 = 0x08;

const char TYPE_INT32 = 0x09;
const char TYPE_UINT32 = 0x0a;

const char TYPE_FLOAT16 = 0x0b;
const char TYPE_FLOAT32 = 0x0c;
const char TYPE_FLOAT64 = 0x0d;

template <class readerT, class writerT>
void copyn(readerT &r, writerT &w, int n) {
  char tmp[32];
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

template <class writerT>
void pack_bytes(writerT &w, const char *s, int n) {
  w.write(s, n);
}

template <class readerT>
void unpack_bytes(readerT &r, char *s, int n) {
  r.read(s, n);
}

template <class writerT>
void pack_uint(writerT &w, unsigned int x, int size) {
  char s[8];

  for (int i = size - 1; i >= 0; i--) {
    s[i] = (char)x;
    x >>= 8;
  }

  pack_bytes(w, s, size);
}

template <class readerT>
unsigned int unpack_uint(readerT &r, int size) {
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
template <class writerT>
void pack_string_val(writerT &w, const char *s) {
  int size = string_size(s, 1024);
  pack_uint(w, TYPE_STRING, 1);
  pack_uint(w, size, 2);
  w.write(s, size);
}

template <class readerT>
void unpack_string_val(readerT &r, char *s) {
  char b[1];

  r.read(b, 1);

  if (b[0] != TYPE_STRING) {
    return;
  }

  while (r.read(b, 1) == 1 && b[0] != '\0') {
    *s++ = b[0];
  }

  *s = '\0';
}

template <class writerT>
void pack_float32(writerT &w, float x) {
  // WARNING Possibly unsafe and unreliable across platforms. Check this
  // carefully!
  const char *b = (const char *)&x;
  pack_bytes(w, b, 4);
}

template <class readerT>
float unpack_float32(readerT &r) {
  char b[4];
  unpack_bytes(r, b, 4);
  return *(const float *)b;
}

template <class writerT>
void pack_float64(writerT &w, double x) {
  // WARNING Possibly unsafe and unreliable across platforms. Check this
  // carefully!
  const char *b = (const char *)&x;
  pack_bytes(w, b, 8);
}

template <class readerT>
double unpack_float64(readerT &r) {
  char b[8];
  unpack_bytes(r, b, 8);
  return *(const double *)b;
}

template <class writerT>
void pack_uint_val(writerT &w, unsigned long x) {
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

template <class writerT>
void pack_float_val(writerT &w, float x) {
  pack_uint(w, TYPE_FLOAT32, 1);
  pack_float32(w, x);
}

template <class writerT>
void pack_double_val(writerT &w, double x) {
  pack_uint(w, TYPE_FLOAT64, 1);
  pack_float64(w, x);
}

template <class readerT>
unsigned long unpack_uint_val(readerT &r) {
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

template <class readerT>
float unpack_float_val(readerT &r) {
  if (unpack_uint(r, 1) != TYPE_FLOAT32) {
    return 0;
  }

  return unpack_float32(r);
}

template <class readerT>
float unpack_double_val(readerT &r) {
  if (unpack_uint(r, 1) != TYPE_FLOAT64) {
    return 0;
  }

  return unpack_float64(r);
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

template <class writerT, class DG>
void pack_datagram(writerT &w, DG &dg) {
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

int find_byte(char x, const char *b, int n) {
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

  char recv_crc = unpack_uint(r, 1);
  char calc_crc = calc_crc8(dg.body.bytes(), dg.body.size());

  if (recv_crc != calc_crc) {
    return false;
  }

  return !r.error();
}

// TODO fix includes so order doesn't matter
#include "sensorgram.h"

#endif
