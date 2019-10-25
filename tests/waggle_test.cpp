#include "waggle.h"
#include <iostream>
#include <sstream>

void test_encode(std::string input, std::string expect) {
  std::stringstream s;

  base64encoder<std::stringstream> e(s);
  e.write(input.c_str(), input.length());
  e.close();

  if (s.str() == expect) {
    std::cout << "PASS encode(\"" << input << "\") = \"" << expect << "\""
              << std::endl;
  } else {
    std::cout << "FAIL encode(\"" << input << "\") -> \"" << s.str()
              << "\" != " << expect << std::endl;
  }
}

void test_simple() {
  base64encoder<typeof(std::cout)> e(std::cout);
  e.write("1234", 4);
  e.close();
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
  test_encode("", "");
  test_encode("A", "QQ");
  test_encode("AZ", "QVo");
  test_encode("AZQ", "QVpR");
  test_encode("1234", "MTIzNA");
  test_encode(wiki_input, wiki_expect);
}
