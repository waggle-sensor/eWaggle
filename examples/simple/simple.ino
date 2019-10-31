#include "waggle.h"

bytebuffer<256> recv_buf;

struct : public writer {
  int write(const byte *s, int n) { return SerialUSB.write(s, n); }
} serial_writer;

void setup() {
  pinMode(13, OUTPUT);

  SerialUSB.begin(9600);

  for (int i = 0; i < 10; i++) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }
}

void loop() {
  while (SerialUSB.available()) {
    int c = SerialUSB.read();

    if (c == '\n') {
      digitalWrite(13, HIGH);
      delay(50);

      show_sensorgram_info();
      recv_buf.reset();

      digitalWrite(13, LOW);
      delay(50);
    } else {
      recv_buf.writebyte(c);
    }
  }
}

// read waiting sensorgrams and output result with incremented timestamp and
// made up data
void show_sensorgram_info() {
  base64_decoder b64d(recv_buf);
  sensorgram_decoder<64> d(b64d);

  // decode sensorgrams in buffer
  while (d.decode()) {
    base64_encoder b64e(serial_writer);
    sensorgram_encoder<64> e(b64e);

    // build sensorgram content
    e.info.timestamp = d.info.timestamp + 1;
    e.info.id = d.info.id;
    e.info.inst = d.info.inst;
    e.info.sub_id = d.info.sub_id;
    e.info.source_id = d.info.source_id;
    e.info.source_inst = d.info.source_inst;

    e.encode_uint(1);
    e.encode_uint(3);
    unsigned int arr1[] = {2, 3, 5, 7};
    e.encode_uint32_array(arr1, 4);
    e.encode_uint(200);
    unsigned int arr2[] = {11, 13, 17};
    e.encode_uint32_array(arr2, 3);
    e.encode();

    b64e.close();
    serial_writer.writebyte('\n');
  }
}
