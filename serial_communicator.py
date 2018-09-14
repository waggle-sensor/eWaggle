import serial

startByte = 0x7e
endByte = 0x7f
escapeByte = 0x7d
escapeXorMask = 0x20


class BlockingCommunicator:

    def __init__(self, device, baudrate=9600, timeout=5000):
        self.serial = serial.Serial(device, baudrate, timeout=timeout)

    def close(self):
        self.serial.close()

    def send(self, message):
        data = []
        data.append(startByte)

        for c in message:
            if c == startByte or c == endByte or c == escapeByte:
                data.append(escapeByte)
                data.append(c ^ escapeXorMask)
            else:
                data.append(c)

        data.append(endByte)

        self.serial.write(bytes(data))

    def receiveByte(self):
        b = self.serial.read(1)

        if not b:
            raise TimeoutError('receiveByte timed out.')

        return b[0]

    def receive(self):
        while True:
            if self.receiveByte() == startByte:
                break

        data = []

        while True:
            c = self.receiveByte()

            if c == endByte:
                break
            elif c == escapeByte:
                c = self.receiveByte()
                data.append(c ^ escapeXorMask)
            else:
                data.append(c)

        return bytes(data)
