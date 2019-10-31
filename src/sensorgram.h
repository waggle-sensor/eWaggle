#ifndef __H_WAGGLE_SENSORGRAM__
#define __H_WAGGLE_SENSORGRAM__

const byte TYPE_NULL = 0x00;

const byte TYPE_BYTE = 0x01;
const byte TYPE_CHAR = 0x02;
const byte TYPE_INT8 = 0x03;
const byte TYPE_UINT8 = 0x04;
const byte TYPE_INT16 = 0x05;
const byte TYPE_UINT16 = 0x06;
const byte TYPE_INT24 = 0x07;
const byte TYPE_UINT24 = 0x08;
const byte TYPE_INT32 = 0x09;
const byte TYPE_UINT32 = 0x0a;
const byte TYPE_FLOAT16 = 0x0b;
const byte TYPE_FLOAT32 = 0x0c;
const byte TYPE_FLOAT64 = 0x0d;

const byte TYPE_BYTE_ARRAY = 0x81;
const byte TYPE_STRING = 0x82;
const byte TYPE_INT8_ARRAY = 0x83;
const byte TYPE_UINT8_ARRAY = 0x84;
const byte TYPE_INT16_ARRAY = 0x85;
const byte TYPE_UINT16_ARRAY = 0x86;
const byte TYPE_INT24_ARRAY = 0x87;
const byte TYPE_UINT24_ARRAY = 0x88;
const byte TYPE_INT32_ARRAY = 0x89;
const byte TYPE_UINT32_ARRAY = 0x8a;
const byte TYPE_FLOAT16_ARRAY = 0x8b;
const byte TYPE_FLOAT32_ARRAY = 0x8c;
const byte TYPE_FLOAT64_ARRAY = 0x8d;

struct sensorgram_info {
  unsigned long timestamp;
  unsigned int id;
  unsigned int inst;
  unsigned int sub_id;
  unsigned int source_id;
  unsigned int source_inst;
};

// TODO think a little more about interface / naming for close / encode

template <int N>
struct sensorgram_encoder {
  sensorgram_info info;
  bytebuffer<N> body;
  writer &w;
  bool err;

  sensorgram_encoder(writer &w) : w(w), err(false) {}

  bool encode() {
    if (err) {
      return false;
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

    // this is ugly. should just have a writeto operation
    e.encode_bytes(body.bytes(), body.size());
    body.clear();

    w.writebyte(crcw.sum);

    // check basic encoder for errors
    if (e.err) {
      err = true;
    }

    return !err;
  }

  void encode_bytes(const byte *s, int n) {
    basic_encoder e(body);
    e.encode_uint(TYPE_BYTE_ARRAY, 1);
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
    const byte *b = (const byte *)&x;
    e.encode_uint(TYPE_FLOAT32, 1);
    e.encode_bytes(b, 4);
  }

  // WARNING Possibly unsafe and unreliable across platforms. Check this
  // carefully!
  void encode_float64(double x) {
    basic_encoder e(body);
    const byte *b = (const byte *)&x;
    e.encode_uint(TYPE_FLOAT64, 1);
    e.encode_bytes(b, 8);
  }

  void encode_uint8_array(const unsigned char s[], int count) {
    basic_encoder e(body);
    e.encode_uint(TYPE_UINT8_ARRAY, 1);
    e.encode_uint(count, 2);

    for (int i = 0; i < count; i++) {
      e.encode_uint(s[i], 1);
    }
  }

  void encode_uint32_array(const unsigned int s[], int count) {
    basic_encoder e(body);
    e.encode_uint(TYPE_UINT32_ARRAY, 1);
    e.encode_uint(count, 2);

    for (int i = 0; i < count; i++) {
      e.encode_uint(s[i], 4);
    }
  }
};

template <int N>
struct sensorgram_decoder {
  sensorgram_info info;
  bytebuffer<N> body;
  reader &r;
  bool err;  // need for encoder too

  sensorgram_decoder(reader &r) : r(r), err(false) {}

  bool decode() {
    if (err) {
      return false;
    }

    crc8_reader crcr(r);

    // read sensorgram content
    basic_decoder d(crcr);
    int len = d.decode_uint(2);
    info.timestamp = d.decode_uint(4);
    info.id = d.decode_uint(2);
    info.inst = d.decode_uint(1);
    info.sub_id = d.decode_uint(1);
    info.source_id = d.decode_uint(2);
    info.source_inst = d.decode_uint(1);
    body.clear();
    body.readfrom(crcr, len);

    // check basic decoder for errors
    if (d.err) {
      err = true;
    }

    // read crc and compare with computed sum
    if (crcr.sum != r.readbyte()) {
      err = true;
    }

    return !err;
  }

  int decode_bytes(byte *s, int max_size) {
    if (err) {
      return 0;
    }

    basic_decoder d(body);

    if (d.decode_uint(1) != TYPE_BYTE_ARRAY) {
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
    byte b[4];
    d.decode_bytes(b, 4);
    return *(const float *)b;
  }

  double decode_float64() {
    if (err) {
      return 0;
    }

    basic_decoder d(body);
    byte b[8];
    d.decode_bytes(b, 8);
    return *(const double *)b;
  }
};

#endif
