#include "waggle.h"
#include <iostream>
#include <sstream>
#include <vector>

template <class T>
bool assert_equal(const T &a, const T &b) {
  bool equal = a == b;

  if (!equal) {
    std::cout << "assert_equal: " << a << " != " << b << std::endl;
  }

  return equal;
}

template <class T>
bool assert_equal_bytes(const T &a, const T &b, int n) {
  bool equal = memcmp(a, b, n) == 0;

  if (!equal) {
    std::cout << "assert_equal_bytes: " << a << " != " << b << std::endl;
  }

  return equal;
}

void check_test(std::string name, bool passed) {
  if (passed) {
    std::cout << "\033[1;32m[PASS]\033[0m " << name << std::endl;
  } else {
    std::cout << "\033[1;31m[FAIL]\033[0m " << name << std::endl;
  }
}

bool test_bytebuffer() {
  bytebuffer<64> b;
  b.write("testing", 7);

  return b.size() == 7;
}

bool test_pack() {
  {
    bytebuffer<64> b;
    pack_bytes(b, "hello", 5);

    if (b.error() || b.size() != 5) {
      return false;
    }

    char s[5];
    unpack_bytes(b, s, 5);

    if (b.error()) {
      return false;
    }

    if (memcmp("hello", s, 5) != 0) {
      return false;
    }
  }

  return true;
}

bool test_pack_uint(const char s[], unsigned int x, int size) {
  bytebuffer<32> b;
  pack_uint(b, x, size);
  return assert_equal(b.size(), size) && assert_equal_bytes(b.bytes(), s, size);
}

bool test_unpack_uint(const char s[], unsigned int x, int size) {
  bytereader r(s, size);
  return assert_equal(unpack_uint(r, size), x);
}

bool test_uint(const char s[], unsigned int x, int size) {
  return test_pack_uint(s, x, size) && test_unpack_uint(s, x, size);
}

bool test_sensorgram() {
  bytebuffer<64> b;
  // sensorgram<32> s;
  // s.timestamp = 1;
  // s.id = 2;
  // s.sub_id = 3;
  // s.source_id = 4;
  // s.source_inst = 5;
  // pack_sensorgram(b, s);

  return false;
}

bool test_base64_encode(std::string input, std::string expect) {
  std::stringstream s;

  base64encoder<std::stringstream> e(s);
  e.write(input.c_str(), input.length());
  e.close();

  return s.str() == expect;
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
    "Vhc3VyZS4";

int main() {
  check_test("bytebuffer", test_bytebuffer());

  check_test("test pack", test_pack());

  check_test("test uint 1", test_uint((const char[]){0x00}, 0x00, 1));
  check_test("test uint 2", test_uint((const char[]){0x12}, 0x12, 1));
  check_test("test uint 3", test_uint((const char[]){0xff}, 0xff, 1));

  // check_test("test uint16 1",
  //            test_unpack_uint16((const char[]){0x00, 0x00}, 0x0000));
  // check_test("test uint16 2",
  //            test_unpack_uint16((const char[]){0x12, 0x34}, 0x1234));
  // check_test("test uint16 3",
  //            test_unpack_uint16((const char[]){0xff, 0xff}, 0xffff));

  // check_test("test uint24 1",
  //            test_unpack_uint24((const char[]){0x00, 0x00, 0x00}, 0x000000));
  // check_test("test uint24 2",
  //            test_unpack_uint24((const char[]){0x12, 0x34, 0x56}, 0x123456));
  // check_test("test uint24 3",
  //            test_unpack_uint24((const char[]){0xff, 0xff, 0xff}, 0xffffff));

  // check_test(
  //     "test uint32 1",
  //     test_unpack_uint32((const char[]){0x00, 0x00, 0x00, 0x00},
  //     0x00000000));

  // check_test(
  //     "test uint32 2",
  //     test_unpack_uint32((const char[]){0x12, 0x34, 0x56, 0x78},
  //     0x12345678));

  // check_test(
  //     "test uint32 3",
  //     test_unpack_uint32((const char[]){0xff, 0xff, 0xff, 0xff},
  //     0xffffffff));

  check_test("base64 empty", test_base64_encode("", ""));
  check_test("base64 1", test_base64_encode("A", "QQ"));
  check_test("base64 2", test_base64_encode("AZ", "QVo"));
  check_test("base64 3", test_base64_encode("AZQ", "QVpR"));
  check_test("base64 4", test_base64_encode("1234", "MTIzNA"));
  check_test("base64 wiki", test_base64_encode(wiki_input, wiki_expect));
}
