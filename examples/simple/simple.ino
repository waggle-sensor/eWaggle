#include "waggle.h"

// serial_writer wraps SerialUSB into the writer interface and ensures
// nonblocking writes
struct : public writer {
  int write(const byte *s, int n) {
    int maxn = SerialUSB.availableForWrite();

    if (n > maxn) {
      n = maxn;
    }

    return SerialUSB.write(s, n);
  }
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

bytebuffer<256> recv_buf;

// loop reads incoming characters off the serial port. if it receives a newline
// then it processes all the messages contained.
void loop() {
  while (SerialUSB.available()) {
    int c = SerialUSB.read();

    if (c == '\n') {
      digitalWrite(13, HIGH);
      delay(50);

      process_messages();
      recv_buf.reset();

      digitalWrite(13, LOW);
      delay(50);
    } else {
      recv_buf.writebyte(c);
    }
  }
}

// process_messages decodes each sensorgram in the buffer and responds
// with incremented timestamp and made up data
void process_messages() {
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

    switch (e.info.id) {
      case 1: {
        e.encode_uint(1);
        e.encode();
      } break;
      case 2: {
        e.encode_uint(1);
        e.encode_uint(2);
        e.encode();
      } break;
      case 3: {
        byte data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        e.encode_bytes(data, 8);
        e.encode();
      } break;
    }

    b64e.close();
    serial_writer.writebyte('\n');
  }
}
