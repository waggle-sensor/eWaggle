#include <cstdarg>
#include <cstdio>
#include "waggle2.h"

int main() {
  Buffer<1024> b;

  {
    Sensorgram<256> sg = {
        .Timestamp = 123456,
        .ID = 123,
        .Inst = 1,
        .SubID = 10,
    };

    PackUintVal(sg.Body, 123);
    PackUintVal(sg.Body, 12345);
    PackUintVal(sg.Body, 1234567);
    PackFloat32(sg.Body, 12.34);

    PackSensorgram(b, sg);
  }

  {
    Sensorgram<256> sg;

    UnpackSensorgram(b, sg);

    printf("%lu\n", UnpackUintVal(sg.Body));
    printf("%lu\n", UnpackUintVal(sg.Body));
    printf("%lu\n", UnpackUintVal(sg.Body));
    printf("%f\n", UnpackFloat32(sg.Body));
  }
}
