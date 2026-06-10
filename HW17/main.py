from machine import Pin, I2C
import time

# =========================
# Pin setup
# =========================

# AS5600 on I2C0
i2c = I2C(0, sda=Pin(4), scl=Pin(5), freq=400000)
AS5600_ADDR = 0x36

# HX711 pins
HX711_DOUT = 14
HX711_SCK = 15


# =========================
# AS5600 functions
# =========================

def scan_i2c():
    devices = i2c.scan()
    print("I2C devices:", [hex(d) for d in devices])
    return devices


def read_as5600_angle():
    """
    Read AS5600 angle register.
    Returns raw angle from 0 to 4095.
    """
    try:
        data = i2c.readfrom_mem(AS5600_ADDR, 0x0E, 2)
        high = data[0]
        low = data[1]
        angle = ((high & 0x0F) << 8) | low
        return angle
    except OSError:
        return -1


def angle_to_degrees(raw_angle):
    if raw_angle < 0:
        return -1
    return raw_angle * 360.0 / 4096.0


# =========================
# HX711 class
# =========================

class HX711:
    def __init__(self, dout_pin, sck_pin, gain=128):
        self.dout = Pin(dout_pin, Pin.IN)
        self.sck = Pin(sck_pin, Pin.OUT)
        self.sck.value(0)

        # gain 128 = 1 extra pulse
        # gain 64  = 3 extra pulses
        # gain 32  = 2 extra pulses
        if gain == 128:
            self.gain_pulses = 1
        elif gain == 64:
            self.gain_pulses = 3
        elif gain == 32:
            self.gain_pulses = 2
        else:
            self.gain_pulses = 1

        self.offset = 0

    def is_ready(self):
        return self.dout.value() == 0

    def read_raw(self, timeout_ms=100):
        """
        Read one raw 24-bit signed value from HX711.
        Returns None if timeout.
        """
        start = time.ticks_ms()

        while self.dout.value() == 1:
            if time.ticks_diff(time.ticks_ms(), start) > timeout_ms:
                return None

        value = 0

        # Read 24 bits
        for _ in range(24):
            self.sck.value(1)
            time.sleep_us(1)

            value = value << 1
            if self.dout.value():
                value += 1

            self.sck.value(0)
            time.sleep_us(1)

        # Extra pulses set gain for next reading
        for _ in range(self.gain_pulses):
            self.sck.value(1)
            time.sleep_us(1)
            self.sck.value(0)
            time.sleep_us(1)

        # Convert 24-bit signed integer
        if value & 0x800000:
            value -= 0x1000000

        return value

    def tare(self, samples=20):
        """
        Set current no-force reading as zero.
        """
        readings = []
        for _ in range(samples):
            r = self.read_raw()
            if r is not None:
                readings.append(r)
            time.sleep_ms(20)

        if len(readings) > 0:
            self.offset = sum(readings) / len(readings)
        else:
            self.offset = 0

        print("HX711 offset:", self.offset)

    def read_force_raw(self):
        r = self.read_raw()
        if r is None:
            return None
        return r - self.offset


# =========================
# Main program
# =========================

hx = HX711(HX711_DOUT, HX711_SCK, gain=128)

print("Starting HW17 Pico sensor reader...")
devices = scan_i2c()

if AS5600_ADDR not in devices:
    print("WARNING: AS5600 not found. Check SDA/SCL/VCC/GND.")
else:
    print("AS5600 found at 0x36.")

print("Do not touch the load cell. Taring HX711...")
time.sleep(1)
hx.tare(samples=20)

print("Start streaming:")
print("angle_raw,angle_deg,force_raw")

while True:
    angle_raw = read_as5600_angle()
    angle_deg = angle_to_degrees(angle_raw)

    force_raw = hx.read_force_raw()
    if force_raw is None:
        force_raw = 0

    # CSV format for computer Python
    print("{},{:.2f},{:.0f}".format(angle_raw, angle_deg, force_raw))

    time.sleep_ms(20)  # about 50 Hz