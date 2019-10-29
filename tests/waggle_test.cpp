#include "waggle.h"
#include <iostream>
#include <sstream>
#include <vector>

// string_writer implements a write buffer on top of the well tested std::string
// to aid our own testing
struct string_writer : public writer {
  std::string str;
  int write(const char *s, int n) {
    str += std::string(s, n);
    return n;
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
  b.write("testing", 7);
  return b.size() == 7;
}

bool test_pack_uint(const char s[], unsigned int x, int size) {
  string_writer w;
  basic_encoder e(w);
  e.encode_uint(x, size);
  return w.str == std::string(s, size);
}

bool test_unpack_uint(const char s[], unsigned int x, int size) {
  bytereader r(s, size);
  basic_decoder d(r);
  return d.decode_uint(size) == x;
}

bool test_uint(const char s[], unsigned int x, int size) {
  return test_pack_uint(s, x, size) && test_unpack_uint(s, x, size);
}

bool test_sensorgram() {
  bytebuffer<64> b;

  {
    sensorgram<32> s;
    s.timestamp = 1;
    s.id = 2;
    s.sub_id = 3;
    s.source_id = 4;
    s.source_inst = 5;
    // pack_uint(s.body, 12, 1);
    // pack_uint(s.body, 3456, 2);
    pack_sensorgram(b, s);
  }

  {
    sensorgram<32> s;

    if (!unpack_sensorgram(b, s)) {
      return false;
    }
    // int a = unpack_uint(s.body, 1);
    // int b = unpack_uint(s.body, 2);
    return (s.timestamp == 1) && (s.id == 2) && (s.sub_id == 3) &&
           (s.source_id == 4) && (s.source_inst == 5);

    // && (a == 12) &&
    //  (b == 3456);
  }
}

bool test_base64_encode(std::string input, std::string expect) {
  string_writer w;
  base64_encoder e(w);
  e.write(input.c_str(), input.length());
  e.close();
  return w.str == expect;
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

  check_test("test uint 1 1", test_uint((const char[]){0x00}, 0x00, 1));
  check_test("test uint 1 2", test_uint((const char[]){0x12}, 0x12, 1));
  check_test("test uint 1 3", test_uint((const char[]){0xff}, 0xff, 1));

  check_test("test uint 2 1", test_uint((const char[]){0x00, 0x00}, 0x0000, 2));
  check_test("test uint 2 2", test_uint((const char[]){0x12, 0x34}, 0x1234, 2));
  check_test("test uint 2 3", test_uint((const char[]){0xff, 0xff}, 0xffff, 2));

  check_test("test uint 3 1",
             test_uint((const char[]){0x00, 0x00, 0x00}, 0x000000, 3));
  check_test("test uint 3 2",
             test_uint((const char[]){0x12, 0x34, 0x56}, 0x123456, 3));
  check_test("test uint 3 3",
             test_uint((const char[]){0xff, 0xff, 0xff}, 0xffffff, 3));

  check_test("test uint 4 1",
             test_uint((const char[]){0x00, 0x00, 0x00, 0x00}, 0x00000000, 4));
  check_test("test uint 4 2",
             test_uint((const char[]){0x12, 0x34, 0x56, 0x78}, 0x12345678, 4));
  check_test("test uint 4 3",
             test_uint((const char[]){0xff, 0xff, 0xff, 0xff}, 0xffffffff, 4));

  check_test("test sensorgram", test_sensorgram());

  check_test("base64 empty", test_base64_encode("", ""));
  check_test("base64 1", test_base64_encode("A", "QQ"));
  check_test("base64 2", test_base64_encode("AZ", "QVo"));
  check_test("base64 3", test_base64_encode("AZQ", "QVpR"));
  check_test("base64 4", test_base64_encode("1234", "MTIzNA"));
  check_test("base64 wiki", test_base64_encode(wiki_input, wiki_expect));
}
