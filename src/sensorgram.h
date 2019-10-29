#ifndef __H_WAGGLE_SENSORGRAM__
#define __H_WAGGLE_SENSORGRAM__

struct sensorgram_info {
  unsigned long timestamp;
  unsigned int id;
  unsigned int inst;
  unsigned int sub_id;
  unsigned int source_id;
  unsigned int source_inst;
};

template <int N>
struct sensorgram_encoder {
  sensorgram_info info;
  bytebuffer<N> body;
  writer &w;
  bool closed;
  bool err;

  sensorgram_encoder(writer &w) : w(w), closed(false) {}

  void close() {
    if (closed) {
      return;
    }

    crc8_writer crcw(w);
    basic_encoder e(crcw);

    e.encode_uint(body.size(), 2);
    e.encode_uint(info.timestamp, 4);
    e.encode_uint(info.id, 2);
    e.encode_uint(info.inst, 1);
    e.encode_uint(info.sub_id, 1);
    e.encode_uint(info.source_id, 2);
    e.encode_uint(info.source_inst, 1);
    e.encode_bytes(body.bytes(), body.size());

    crcw.close();
  }

  void encode_bytes(const char *s, int n) {
    basic_encoder e(body);
    e.encode_uint(TYPE_BYTES, 1);
    e.encode_uint(n, 2);
    e.encode_bytes(s, n);
  }

  void encode_uint(unsigned long x) {
    basic_encoder e(body);

    if (x <= 0xff) {
      e.encode_uint(TYPE_UINT8, 1);
      e.encode_uint(x, 1);
      return;
    }

    if (x <= 0xffff) {
      e.encode_uint(TYPE_UINT16, 1);
      e.encode_uint(x, 2);
      return;
    }

    if (x <= 0xffffff) {
      e.encode_uint(TYPE_UINT24, 1);
      e.encode_uint(x, 3);
      return;
    }

    e.encode_uint(TYPE_UINT32, 1);
    e.encode_uint(x, 4);
  }

  // WARNING Possibly unsafe and unreliable across platforms. Check this
  // carefully!
  void encode_float32(float x) {
    basic_encoder e(body);
    const char *b = (const char *)&x;
    e.encode_bytes(b, 4);
  }

  // WARNING Possibly unsafe and unreliable across platforms. Check this
  // carefully!
  void pack_float64(double x) {
    basic_encoder e(body);
    const char *b = (const char *)&x;
    e.encode_bytes(b, 8);
  }
};

template <int N>
struct sensorgram_decoder {
  sensorgram_info info;
  bytebuffer<N> body;
  reader &r;
  bool err;  // need for encoder too

  sensorgram_decoder(reader &r) : r(r) {
    crc8_reader crcr(r);
    basic_decoder d(r);

    // read sensorgram content
    int len = d.decode_uint(2);
    info.timestamp = d.decode_uint(4);
    info.id = d.decode_uint(2);
    info.inst = d.decode_uint(1);
    info.sub_id = d.decode_uint(1);
    info.source_id = d.decode_uint(2);
    info.source_inst = d.decode_uint(1);
    body.readfrom(crcr, len);

    // throw away trailing crc byte and check for errors
    d.decode_uint(1);

    err = d.err || (crcr.sum != 0);
  }
};

template <class readerT>
float unpack_float32(readerT &r) {
  char b[4];
  unpack_bytes(r, b, 4);
  return *(const float *)b;
}

template <class readerT>
double unpack_float64(readerT &r) {
  char b[8];
  unpack_bytes(r, b, 8);
  return *(const double *)b;
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
