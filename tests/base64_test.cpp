#include <iostream>
#include <sstream>
#include "waggle.h"

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

int main() {
  test_encode("", "");
  test_encode("A", "QQ");
  test_encode("AZ", "QVo");
  test_encode("1234", "MTIzNA");
}
