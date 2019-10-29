#ifndef __H_WAGGLE_SENSORGRAM__
#define __H_WAGGLE_SENSORGRAM__

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

// sure, we could move the crc out of this too since it's primarily used for
// transmission. that would clean up the abstraction a bit.
//
// of course, we can just move to a sensorgram_encoder / decoder
// that's just a composition of these two.
//
// these could also track the various errors we run into.

template <class SG>
void pack_sensorgram(writer &w, SG &sg) {
  crc8_writer crcw(w);
  basic_encoder e(crcw);

  // write sensorgram content
  e.encode_uint(sg.body.size(), 2);
  e.encode_uint(sg.timestamp, 4);
  e.encode_uint(sg.id, 2);
  e.encode_uint(sg.inst, 1);
  e.encode_uint(sg.sub_id, 1);
  e.encode_uint(sg.source_id, 2);
  e.encode_uint(sg.source_inst, 1);
  e.encode_bytes(sg.body.bytes(), sg.body.size());

  // write crc sum
  crcw.close();
}

template <class SG>
bool unpack_sensorgram(reader &r, SG &sg) {
  crc8_reader crcr(r);
  basic_decoder d(r);

  // read sensorgram content
  int len = d.decode_uint(2);
  sg.timestamp = d.decode_uint(4);
  sg.id = d.decode_uint(2);
  sg.inst = d.decode_uint(1);
  sg.sub_id = d.decode_uint(1);
  sg.source_id = d.decode_uint(2);
  sg.source_inst = d.decode_uint(1);

  // this is weird...
  sg.body.clear();
  copyn(crcr, sg.body, len);

  // throw away trailing crc byte and check for errors
  d.decode_uint(1);
  return !d.err && crcr.sum == 0;
}

void pack_bytes_val(writer &w, const char *s, int n) {
  basic_encoder e(w);
  e.encode_uint(TYPE_BYTES, 1);
  e.encode_uint(n, 2);
  e.encode_bytes(s, n);
}

// template <class writerT>
// void pack_string_val(writerT &w, const char *s) {
//   int size = string_size(s, 1024);
//   pack_uint(w, TYPE_STRING, 1);
//   pack_uint(w, size, 2);
//   w.write(s, size);
// }

// need a value reader / writer too to manage error handling...

// template <class readerT>
// void unpack_string_val(readerT &r, char *s) {
//   int type = unpack_uint(r, 1);

//   // need a way to signal error
//   if (type != TYPE_STRING) {
//     return;
//   }

//   while (r.read(b, 1) == 1 && b[0] != '\0') {
//     *s++ = b[0];
//   }

//   *s = '\0';
// }

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

#endif
