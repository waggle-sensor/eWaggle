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
  // example: appending one sensorgram to a datagram
  {
    Datagram<256> dg;
    dg.PluginID = 1;
    dg.PluginMajorVersion = 1;
    dg.PluginMinorVersion = 0;
    dg.PluginPatchVersion = 0;
    dg.Timestamp = 111111;

    Sensorgram<32> sg;
    sg.Timestamp = 123456;
    sg.ID = 1;
    sg.Inst = 1;
    sg.SubID = 10;
    sg.SourceID = 1;
    sg.SourceInst = 2;

    // append sensorgram values
    PackUintVal(sg.Body, 123);
    PackFloatVal(sg.Body, 12.34);
    PackSensorgram(dg.Body, sg);

    // write datagram to loopback buffer
    PackDatagram(loopBuf, dg);
  }

  // example: appending two sensorgrams to a datagram
  {
    Datagram<256> dg;
    dg.PluginMajorVersion = 1;
    dg.PluginMinorVersion = 0;
    dg.PluginPatchVersion = 0;
    dg.Timestamp = 111111;

    {
      Sensorgram<32> sg;
      sg.Timestamp = 123456;
      sg.ID = 2;
      sg.Inst = 1;
      sg.SubID = 10;
      sg.SourceID = 1;
      sg.SourceInst = 2;

      PackUintVal(sg.Body, 123);
      PackUintVal(sg.Body, 12345);

      PackSensorgram(dg.Body, sg);
    }

    {
      Sensorgram<32> sg;
      sg.Timestamp = 123456;
      sg.ID = 3;
      sg.Inst = 1;
      sg.SubID = 10;
      sg.SourceID = 1;
      sg.SourceInst = 2;

      PackUintVal(sg.Body, 1);

      PackSensorgram(dg.Body, sg);
    }

    // write datagram to loopback buffer
    PackDatagram(loopBuf, dg);
  }

  // example: unpacking all datagrams in loopback buffer
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
