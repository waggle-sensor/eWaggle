#include "waggle/waggle_native.h"

using namespace waggle;

Buffer<256> loopbackBuffer;
Messenger<256> messenger(loopbackBuffer);

// Messenger<256> messenger(serialIO);
Plugin<256> plugin(37, 2, 0, 0, 0);

int main() {
    // publisher
    plugin.AddMeasurement(1, 0, 0, 0, (byte *)"first", 5);
    plugin.AddMeasurement(2, 0, 0, 0, (byte *)"second", 6);

    messenger.StartMessage();
    plugin.PublishMeasurements(messenger);
    messenger.EndMessage();

    plugin.AddMeasurement(3, 0, 1, 0, (byte *)"123", 3);
    plugin.AddMeasurement(4, 3, 1, 0, (byte *)"4", 1);
    plugin.AddMeasurement(9, 3, 2, 0, (byte *)"4", 1);

    messenger.StartMessage();
    plugin.PublishMeasurements(messenger);
    messenger.EndMessage();

    // processor
    while (messenger.ReadMessage()) {
        MessageScanner<64> scanner(messenger.Message());

        while (scanner.ScanDatagram()) {
            printf("datagram %d\n", scanner.Datagram().pluginID);

            while (scanner.ScanSensorgram()) {
                printf("sensorgram %d %d\n", scanner.Sensorgram().sensorID, scanner.Sensorgram().parameterID);
            }
        }
    }
}
