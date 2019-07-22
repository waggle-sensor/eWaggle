#include <cstdio>
#include "Waggle.h"

Buffer<1024> b;

void PackExample() {
  Sensorgram<256> sg = {
      .Timestamp = 123456,
      .ID = 123,
      .Inst = 1,
      .SubID = 10,
      .SourceID = 1,
      .SourceInst = 2,
  };

  PackUintVal(sg.Body, 123);
  PackUintVal(sg.Body, 12345);
  PackUintVal(sg.Body, 1234567);
  PackFloat32(sg.Body, 12.34);

  PackSensorgram(b, sg);
}

void UnpackExample() {
  Sensorgram<256> sg;

  UnpackSensorgram(b, sg);

  printf("Timestamp = %lu\n", sg.Timestamp);
  printf("ID = %u\n", sg.ID);
  printf("Inst = %u\n", sg.Inst);
  printf("SubID = %u\n", sg.SubID);
  printf("SourceID = %u\n", sg.SourceID);
  printf("SourceInst = %u\n", sg.SourceInst);

  printf("%lu\n", UnpackUintVal(sg.Body));
  printf("%lu\n", UnpackUintVal(sg.Body));
  printf("%lu\n", UnpackUintVal(sg.Body));
  printf("%f\n", UnpackFloat32(sg.Body));
}

int main() {
  PackExample();
  UnpackExample();
}
