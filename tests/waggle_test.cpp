#include "waggle.h"
#include <iostream>
#include <sstream>

template <class T>
bool assert_equal(const T &a, const T &b) {
  bool equal = a == b;

  if (!equal) {
    std::cout << "assert_equal: " << a << " != " << b << std::endl;
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

bool test_unpack_uint8(const char *s, unsigned int x) {
  bytereader r(s, 1);
  return assert_equal(unpack_uint8(r), x);
}

bool test_unpack_uint16(const char *s, unsigned int x) {
  bytereader r(s, 2);
  return assert_equal(unpack_uint16(r), x);
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

  check_test("test uint8 0", test_unpack_uint8((const char[]){0x00}, 0x00));
  check_test("test uint8 100", test_unpack_uint8((const char[]){0x12}, 0x12));
  check_test("test uint8 255", test_unpack_uint8((const char[]){0xff}, 0xff));

  check_test("test uint16 1",
             test_unpack_uint16((const char[]){0x00, 0x00}, 0x00));
  check_test("test uint16 2",
             test_unpack_uint16((const char[]){0x12, 0x34}, 0x1234));
  check_test("test uint16 3",
             test_unpack_uint16((const char[]){0xff, 0xff}, 0xffff));

  check_test("base64 empty", test_base64_encode("", ""));
  check_test("base64 1", test_base64_encode("A", "QQ"));
  check_test("base64 2", test_base64_encode("AZ", "QVo"));
  check_test("base64 3", test_base64_encode("AZQ", "QVpR"));
  check_test("base64 4", test_base64_encode("1234", "MTIzNA"));
  check_test("base64 wiki", test_base64_encode(wiki_input, wiki_expect));
}
