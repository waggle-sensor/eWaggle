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
    encode_content(crcw);
    w.writebyte(crcw.sum);
  }

  void encode_content(crc8_writer &crcw) {
    basic_encoder e(crcw);
    e.encode_uint(body.size(), 2);
    e.encode_uint(info.timestamp, 4);
    e.encode_uint(info.id, 2);
    e.encode_uint(info.inst, 1);
    e.encode_uint(info.sub_id, 1);
    e.encode_uint(info.source_id, 2);
    e.encode_uint(info.source_inst, 1);
    e.encode_bytes(body.bytes(), body.size());
  }

  void encode_bytes(const char *s, int n) {
    basic_encoder e(body);
    e.encode_uint(TYPE_BYTES, 1);
    e.encode_uint(n, 2);
    e.encode_bytes(s, n);
  }

  void encode_uint(unsigned int x) {
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
  void encode_float64(double x) {
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
    decode_content(crcr);
    char crc = r.readbyte();
    err = crcr.sum != crc;
  }

  void decode_content(crc8_reader &crcr) {
    basic_decoder d(crcr);
    int len = d.decode_uint(2);
    info.timestamp = d.decode_uint(4);
    info.id = d.decode_uint(2);
    info.inst = d.decode_uint(1);
    info.sub_id = d.decode_uint(1);
    info.source_id = d.decode_uint(2);
    info.source_inst = d.decode_uint(1);
    body.readfrom(crcr, len);
  }

  int decode_bytes(char *s, int max_size) {
    if (err) {
      return 0;
    }

    basic_decoder d(body);

    if (d.decode_uint(1) != TYPE_BYTES) {
      err = true;
      return 0;
    }

    int size = d.decode_uint(2);

    if (size > max_size) {
      err = true;
      size = max_size;
    }

    d.decode_bytes(s, size);

    if (d.err) {
      err = true;
    }

    return size;
  }

  unsigned int decode_uint() {
    if (err) {
      return 0;
    }

    basic_decoder d(body);
    unsigned int x;

    switch (d.decode_uint(1)) {
      case TYPE_UINT8:
        x = d.decode_uint(1);
        break;
      case TYPE_UINT16:
        x = d.decode_uint(2);
        break;
      case TYPE_UINT24:
        x = d.decode_uint(3);
        break;
      case TYPE_UINT32:
        x = d.decode_uint(4);
        break;
      default:
        err = true;
        break;
    }

    if (d.err) {
      err = true;
    }

    if (err) {
      return 0;
    }

    return x;
  }

  float decode_float32() {
    if (err) {
      return 0;
    }

    basic_decoder d(body);
    char b[4];
    d.decode_bytes(b, 4);
    return *(const float *)b;
  }

  double decode_float64() {
    if (err) {
      return 0;
    }

    basic_decoder d(body);
    char b[8];
    d.decode_bytes(b, 8);
    return *(const double *)b;
  }
};

#endif
