import serial
import time
import glob
import numpy as np
import matplotlib.pyplot as plt

# Number of samples to collect
N = 800
BAUD = 115200


def find_pico_port():
    ports = glob.glob("/dev/tty.usbmodem*") + glob.glob("/dev/cu.usbmodem*")

    if len(ports) == 0:
        raise RuntimeError(
            "No Pico serial port found. "
            "Unplug/replug Pico without BOOTSEL, then try again."
        )

    return ports[0]


PORT = find_pico_port()
print("Using port:", PORT)

ser = serial.Serial(PORT, BAUD, timeout=10)
time.sleep(2)
ser.reset_input_buffer()

# Send sample number to Pico
ser.write(f"{N}\n".encode())

lines = []

# Wait for Pico to say BEGIN
while True:
    line = ser.readline().decode(errors="ignore").strip()
    print(line)

    if line.startswith("BEGIN"):
        break

# Read header line
header = ser.readline().decode(errors="ignore").strip()
print(header)

# Read data
while True:
    line = ser.readline().decode(errors="ignore").strip()

    if line == "END":
        break

    if line:
        lines.append(line)

ser.close()

if len(lines) == 0:
    raise RuntimeError("No data received from Pico.")

# Convert data to numpy array
data = np.array([[float(x) for x in line.split(",")] for line in lines])

t_ms = data[:, 0]
raw = data[:, 1]
filt = data[:, 2]

# Convert time to seconds, starting from 0
t = (t_ms - t_ms[0]) / 1000.0

# Save CSV data
np.savetxt(
    "hw14_data.csv",
    data,
    delimiter=",",
    header="time_ms,raw,filtered",
    comments=""
)

# Time-domain plot
plt.figure()
plt.plot(t, raw, label="Raw")
plt.plot(t, filt, label="IIR filtered")
plt.xlabel("Time (s)")
plt.ylabel("HX711 value")
plt.title("Force Sensor Raw and Filtered Data")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("hw14_time_data.png", dpi=300)
plt.show()

# FFT
dt = np.mean(np.diff(t))
fs = 1.0 / dt

raw_zero = raw - np.mean(raw)
filt_zero = filt - np.mean(filt)

freq = np.fft.rfftfreq(len(t), d=dt)
raw_fft = np.abs(np.fft.rfft(raw_zero))
filt_fft = np.abs(np.fft.rfft(filt_zero))

plt.figure()
plt.plot(freq, raw_fft, label="Raw FFT")
plt.plot(freq, filt_fft, label="Filtered FFT")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude")
plt.title(f"FFT of Force Sensor Data, Fs = {fs:.1f} Hz")
plt.xlim(0, 40)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("hw14_fft.png", dpi=300)
plt.show()

print("Done.")
print("Saved: hw14_data.csv")
print("Saved: hw14_time_data.png")
print("Saved: hw14_fft.png")