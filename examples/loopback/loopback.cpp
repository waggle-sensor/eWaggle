#include <Waggle.h>

using namespace Waggle;

int main() {
  // First, we define a loopback device with 256 bytes of scratch space on the
  // stack.
  //
  // A LoopbackIO device is a helper object which "loops" writes back to reads.
  // After testing, other IO devices such as SerialIO can be used as a drop-in
  // replacement.
  LoopbackIO<1024> io;

  // Now, we define a sensorgram with 64 bytes of scratch space on the stack.
  Sensorgram<64> s;

  // In this section, we pack a bunch of test sensorgrams into our loopback
  // device.
  s.sensorID = 3;
  s.parameterID = 1;
  s.sensorInstance = 1;
  s.PackInt(100);
  s.PackInt(200);
  s.Pack(io);

  s.sensorID = 4;
  s.parameterID = 1;
  s.sensorInstance = 1;
  s.PackInt(123);
  s.Pack(io);

  s.sensorID = 5;
  s.parameterID = 1;
  s.sensorInstance = 1;
  s.PackInt(456);
  s.Pack(io);

  // Finally, we re-unpack the sensorgrams from our loopback device and print
  // some basic info about them.
  while (s.Unpack(io)) {
    printf("sensor id: %d\n", s.sensorID);
    printf("parameter id: %d\n", s.parameterID);
    printf("value: %d\n", s.UnpackInt());
    printf("value: %d\n", s.UnpackInt());
    printf("\n");
  }

  return 0;
}
