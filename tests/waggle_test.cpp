#include "waggle.h"
#include <iostream>
#include <sstream>
#include <vector>

struct : public writer {
  int write(const byte *s, int n) {
    std::cout << std::string((const char *)s, n);
    return n;
  }

} cout_writer;

struct bytereader : public reader {
  const byte *buf;
  int size;
  int pos;

  bytereader(const byte buf[], int size) : buf(buf), size(size), pos(0) {}

  int read(byte *s, int n) {
    int i = 0;

    while (i < n && pos < size) {
      s[i++] = buf[pos++];
    }

    return i;
  }
};

// string_writer implements a write buffer on top of the well tested std::string
// to aid our own testing
struct string_buffer : public writer, public reader {
  std::string str;

  int write(const byte *s, int n) {
    str.append((const char *)s, n);
    return n;
  }

  int read(byte *s, int n) {
    int nread = str.copy((char *)s, n);
    str = str.substr(nread);
    return nread;
  }
};

void check_test(std::string name, bool passed) {
  if (passed) {
    std::cout << "\033[1;32m[PASS]\033[0m " << name << std::endl;
  } else {
    std::cout << "\033[1;31m[FAIL]\033[0m " << name << std::endl;
  }
}

bool test_bytebuffer() {
  bytebuffer<64> b;
  b.write((const byte *)"testing", 7);
  return b.size() == 7;
}

bool test_pack_uint(const byte s[], unsigned int x, int size) {
  string_buffer w;
  basic_encoder e(w);
  e.encode_uint(x, size);
  return w.str == std::string((const char *)s, size);
}

bool test_unpack_uint(const byte s[], unsigned int x, int size) {
  bytebuffer<64> r(s, size);
  basic_decoder d(r);
  return d.decode_uint(size) == x;
}

bool test_uint(const byte s[], unsigned int x, int size) {
  return test_pack_uint(s, x, size) && test_unpack_uint(s, x, size);
}

bool test_sensorgram() {
  string_buffer b;

  sensorgram_encoder<64> e(b);
  e.info.timestamp = 1;
  e.info.id = 2;
  e.info.sub_id = 3;
  e.info.source_id = 4;
  e.info.source_inst = 5;
  e.encode_uint(6);
  e.encode_uint(700);
  e.encode_uint(80000);
  e.close();

  sensorgram_decoder<64> d(b);

  if (d.decode_uint() != 6) {
    return false;
  }

  if (d.decode_uint() != 700) {
    return false;
  }

  if (d.decode_uint() != 80000) {
    return false;
  }

  return (d.err == false) && (e.info.timestamp == d.info.timestamp) &&
         (e.info.id == d.info.id) && (e.info.sub_id == d.info.sub_id) &&
         (e.info.source_id == d.info.source_id) &&
         (e.info.source_inst == d.info.source_inst);
}

bool test_base64_encode(std::string input, std::string expect) {
  string_buffer w;
  base64_encoder e(w);
  e.write((const byte *)input.c_str(), input.length());
  e.close();
  return w.str == expect;
}

bool test_crc_value() {
  string_buffer b;
  crc8_writer w(b);
  w.write((const byte[]){1, 2, 3, 4, 5}, 5);
  return w.sum == (byte)42;
}

bool test_crc(std::string input) {
  string_buffer b;

  crc8_writer w(b);
  w.write((const byte *)input.c_str(), input.length());
  b.writebyte(w.sum);

  if (b.str.length() != input.length() + 1) {
    std::cout << "crc wrong size" << std::endl;
    return false;
  }

  crc8_reader r(b);
  byte tmp[256];
  r.read(tmp, input.length());

  if (r.sum != b.readbyte()) {
    std::cout << "crc wrong value" << std::endl;
    return false;
  }

  return true;
}

bool test_base64_decode() {
  // base64 encoding of {0, 1, ..., 255}
  const byte testdata[] =
      "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMz"
      "Q1Njc4OTo7PD0+"
      "P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3Bxcn"
      "N0dXZ3eHl6e3x9fn+"
      "AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+"
      "wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/"
      "g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==";

  bytereader r(testdata, sizeof(testdata));
  base64_decoder d(r);

  // check that we've reconstructed {0, 1, ..., 255}
  for (int i = 0; i < 256; i++) {
    if (d.readbyte() != i) {
      return false;
    }
  }

  return true;
}

std::string wiki_input =
    "Man is distinguished, not only by his reason, but by this singular "
    "passion from other animals, "
    "which is a lust of the mind, that by a perseverance of delight in the "
    "continued and indefatigable "
    "generation of knowledge, exceeds the short vehemence of any carnal "
    "pleasure.";

std::string wiki_expect =
    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieS"
    "B0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBh"
    "IGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodC"
    "BpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25v"
    "d2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbG"
    "Vhc3VyZS4=";

int main() {
  check_test("bytebuffer", test_bytebuffer());

  check_test("test uint 1 1", test_uint((const byte[]){0x00}, 0x00, 1));
  check_test("test uint 1 2", test_uint((const byte[]){0x12}, 0x12, 1));
  check_test("test uint 1 3", test_uint((const byte[]){0xff}, 0xff, 1));

  check_test("test uint 2 1", test_uint((const byte[]){0x00, 0x00}, 0x0000, 2));
  check_test("test uint 2 2", test_uint((const byte[]){0x12, 0x34}, 0x1234, 2));
  check_test("test uint 2 3", test_uint((const byte[]){0xff, 0xff}, 0xffff, 2));

  check_test("test uint 3 1",
             test_uint((const byte[]){0x00, 0x00, 0x00}, 0x000000, 3));
  check_test("test uint 3 2",
             test_uint((const byte[]){0x12, 0x34, 0x56}, 0x123456, 3));
  check_test("test uint 3 3",
             test_uint((const byte[]){0xff, 0xff, 0xff}, 0xffffff, 3));

  check_test("test uint 4 1",
             test_uint((const byte[]){0x00, 0x00, 0x00, 0x00}, 0x00000000, 4));
  check_test("test uint 4 2",
             test_uint((const byte[]){0x12, 0x34, 0x56, 0x78}, 0x12345678, 4));
  check_test("test uint 4 3",
             test_uint((const byte[]){0xff, 0xff, 0xff, 0xff}, 0xffffffff, 4));

  check_test("test sensorgram", test_sensorgram());

  check_test("base64 empty", test_base64_encode("", ""));
  check_test("base64 1", test_base64_encode("A", "QQ=="));
  check_test("base64 2", test_base64_encode("AZ", "QVo="));
  check_test("base64 3", test_base64_encode("AZQ", "QVpR"));
  check_test("base64 4", test_base64_encode("1234", "MTIzNA=="));
  check_test("base64 wiki", test_base64_encode(wiki_input, wiki_expect));

  check_test("crc value", test_crc_value());
  check_test("crc", test_crc("hello"));

  {
    unsigned int values1[5] = {1, 2, 3, 4, 5};
    unsigned int values2[4] = {6, 7, 8, 9};

    base64_encoder textw(cout_writer);
    sensorgram_encoder<256> e(textw);
    e.info.timestamp = 0x11111111;
    e.info.id = 0x2222;
    e.info.inst = 0x33;
    e.info.sub_id = 0x44;
    e.info.source_id = 0x5555;
    e.info.source_inst = 0x66;
    e.encode_uint(13);
    e.encode_uint(17);
    e.encode_uint(1001);
    e.encode_uint32_array(values1, 5);
    e.encode_uint32_array(values2, 4);
    e.close();
    textw.close();
    std::cout << std::endl;
  }

  check_test("base64 decode", test_base64_decode());

  {
    const byte line[] = "AAcAqYrHCK4hLBWzQgQHBFgGJw9s";
    bytereader r(line, sizeof(line));
    base64_decoder b64(r);

    sensorgram_decoder<256> d(b64);

    std::cout << d.info.timestamp << std::endl
              << d.info.id << std::endl
              << d.info.inst << std::endl
              << d.info.sub_id << std::endl;

    std::cout << d.decode_uint() << std::endl;
    std::cout << d.decode_uint() << std::endl;
    std::cout << d.decode_uint() << std::endl;
  }
}
