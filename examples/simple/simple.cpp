#include <cstdio>
#include "Waggle.h"

Buffer<1024> loopBuf;

void printHex(const unsigned char *b, int n) {
  for (int i = 0; i < n; i++) {
    printf("%02x", b[i]);
  }

  printf("\n");
}

int main() {
  // Test Datagram 1
  {
    Datagram<256> dg = {
        .Timestamp = 111111,
    };

    Sensorgram<32> sg = {
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
    PackSensorgram(dg.Body, sg);
    PackDatagram(loopBuf, dg);
  }

  {
    Datagram<256> dg = {
        .Timestamp = 111111,
    };

    {
      Sensorgram<32> sg = {
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
      PackSensorgram(dg.Body, sg);
    }

    {
      Sensorgram<32> sg = {
          .Timestamp = 123456,
          .ID = 123,
          .Inst = 1,
          .SubID = 10,
          .SourceID = 1,
          .SourceInst = 2,
      };

      PackUintVal(sg.Body, 1);
      PackSensorgram(dg.Body, sg);
    }

    PackDatagram(loopBuf, dg);
  }

  {
    Datagram<256> dg;

    while (UnpackDatagram(loopBuf, dg)) {
      printf("datagram\n");
      Sensorgram<32> sg;

      while (UnpackSensorgram(dg.Body, sg)) {
        printf(" sensorgram\n");
      }
    }
  }
}
