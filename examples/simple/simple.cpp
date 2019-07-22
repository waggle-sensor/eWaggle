#include <cstdio>
#include "Waggle.h"

Buffer<1024> loopBuf;

void PackExample() {
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

void printHex(const unsigned char *b, int n) {
  for (int i = 0; i < n; i++) {
    printf("%02x", b[i]);
  }

  printf("\n");
}

int compareBytes(const unsigned char *a, const unsigned char *b, int n) {
  for (int i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return false;
    }
  }

  return true;
}

void testBuffer() {
  Buffer<32> b;
  const unsigned char tmp[] = {1, 2, 3};

  if (b.Write(tmp, 3) != 3) {
    printf("Buffer.Write failed\n");
    return;
  }

  if (b.Len() != 3) {
    printf("Buffer.Len failed\n");
    return;
  }

  if (!compareBytes(tmp, b.Bytes(), b.Len())) {
    printf("Buffer.Bytes failed\n");
    return;
  }
}

int main() {
  testBuffer();

  PackExample();
  PackExample();
  PackExample();

  Datagram<256> dg;

  for (int i = 0; i < 10; i++) {
    printf("--- buffer state %d\n", i);
    printHex(loopBuf.Bytes(), loopBuf.Len());

    if (!UnpackDatagram(loopBuf, dg)) {
      break;
    }

    printHex(dg.Body.Bytes(), dg.Body.Len());
  }
}
