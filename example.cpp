#include "waggle/waggle_native.h"

using namespace waggle;

Buffer<256> loopbackBuffer;
Messenger<256> messenger(loopbackBuffer);
Plugin<256> plugin(37, 2, 0, 0, 0);

void testPublish() {
    plugin.AddMeasurement(1, 0, 0, 0, (byte *)"first", 5);
    plugin.AddMeasurement(2, 0, 0, 0, (byte *)"second", 6);
    plugin.PublishMeasurements(messenger);

    plugin.AddMeasurement(3, 0, 1, 0, (byte *)"123", 3);
    plugin.AddMeasurement(4, 3, 1, 0, (byte *)"4", 1);
    plugin.AddMeasurement(9, 3, 2, 0, (byte *)"4", 1);
    plugin.PublishMeasurements(messenger);
}

void testProcess() {
    while (messenger.ReadMessage()) {
        MessageScanner<64> scanner(messenger.Message());

        while (scanner.ScanDatagram()) {
            const DatagramInfo &datagram = scanner.Datagram();
            printf("datagram %d\n", datagram.pluginID);

            while (scanner.ScanSensorgram()) {
                const SensorgramInfo &sensorgram = scanner.Sensorgram();
                printf("sensorgram %d %d\n", sensorgram.sensorID, sensorgram.parameterID);
            }
        }
    }
}

int main() {
    testPublish();
    testProcess();
}
