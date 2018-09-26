
using namespace waggle;

StreamIO systemIO(SerialUSB);
Messenger<256> messenger(systemIO);

void setup() {
    SerialUSB.begin(9600);
    SerialUSB.setTimeout(1000);
}

void loop() {
    Sensorgram<32> req;
    Sensorgram<32> resp;

    while (messenger.ReadMessage()) {
        while (req.Unpack(messenger.Message())) {
            if (req.sensorID == 1) {
                resp.sensorID = 0x81;
                resp.parameterID = 0;
                resp.SetString("alive");
            } else if (req.sensorID == 2) {
                resp.sensorID = 0x82;
                resp.parameterID = 0;
                resp.SetUint(0);
            } else {
                continue;
            }

            messenger.StartMessage();
            resp.Pack(messenger)
            messenger.EndMessage();
        }
    }
}
