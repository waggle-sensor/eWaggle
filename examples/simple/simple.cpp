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
        .ID = 1,
        .Inst = 1,
        .SubID = 10,
        .SourceID = 1,
        .SourceInst = 2,
    };

    PackUintVal(sg.Body, 123);
    PackFloatVal(sg.Body, 12.34);
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
          .ID = 2,
          .Inst = 1,
          .SubID = 10,
          .SourceID = 1,
          .SourceInst = 2,
      };

      PackUintVal(sg.Body, 123);
      PackUintVal(sg.Body, 12345);
      PackSensorgram(dg.Body, sg);
    }

    {
      Sensorgram<32> sg = {
          .Timestamp = 123456,
          .ID = 3,
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
        printf(" sensorgram ID %d\n", sg.ID);

        switch (sg.ID) {
          case 1: {
            unsigned int x = UnpackUintVal(sg.Body);
            float y = UnpackFloatVal(sg.Body);
            printf("  values: %u %f\n", x, y);
          } break;
          case 2: {
            unsigned int x = UnpackUintVal(sg.Body);
            unsigned int y = UnpackUintVal(sg.Body);
            printf("  values: %u %u\n", x, y);
          } break;
          case 3: {
            unsigned int x = UnpackUintVal(sg.Body);
            printf("  values: %u\n", x);
          } break;
        }
      }
    }
  }
}
