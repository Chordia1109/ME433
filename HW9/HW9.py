import csv
import numpy as np
import matplotlib.pyplot as plt


def read_csv(filename):
    t = []
    signal = []

    with open(filename, newline='') as f:
        reader = csv.reader(f)

        for row in reader:
            try:
                t.append(float(row[0]))
                signal.append(float(row[1]))
            except:
                # skip header or bad rows
                pass

    return np.array(t), np.array(signal)


def get_sample_rate(t):
    total_time = t[-1] - t[0]
    return len(t) / total_time


def get_fft(signal, sample_rate):
    n = len(signal)
    freqs = np.fft.rfftfreq(n, d=1 / sample_rate)
    fft_mag = np.abs(np.fft.rfft(signal))
    return freqs, fft_mag


def moving_average(signal, X):
    filtered = []

    for i in range(len(signal)):
        if i < X:
            avg = np.mean(signal[0:i + 1])
        else:
            avg = np.mean(signal[i - X + 1:i + 1])

        filtered.append(avg)

    return np.array(filtered)


def iir_filter(signal, A, B):
    filtered = np.zeros(len(signal))
    filtered[0] = signal[0]

    for i in range(1, len(signal)):
        filtered[i] = A * filtered[i - 1] + B * signal[i]

    return filtered


def make_fir_weights(cutoff_hz, sample_rate, num_weights):
    # normalized cutoff frequency
    fc = cutoff_hz / sample_rate

    # symmetric index
    n = np.arange(num_weights)
    middle = (num_weights - 1) / 2

    # low-pass sinc filter
    h = 2 * fc * np.sinc(2 * fc * (n - middle))

    # Hamming window
    window = np.hamming(num_weights)
    h = h * window

    # normalize so low frequency gain is about 1
    h = h / np.sum(h)

    return h


def fir_filter(signal, weights):
    filtered = np.zeros(len(signal))

    for i in range(len(signal)):
        total = 0

        for j in range(len(weights)):
            if i - j >= 0:
                total += weights[j] * signal[i - j]

        filtered[i] = total

    return filtered


def plot_time_fft(filename):
    t, signal = read_csv(filename)
    sample_rate = get_sample_rate(t)
    print(filename, "sample rate =", sample_rate, "Hz")

    freqs, fft_mag = get_fft(signal, sample_rate)

    plt.figure(figsize=(10, 6))

    plt.subplot(2, 1, 1)
    plt.plot(t, signal, 'k')
    plt.xlabel("Time (s)")
    plt.ylabel("Signal")
    plt.title(filename + " Signal vs Time")

    plt.subplot(2, 1, 2)
    plt.plot(freqs, fft_mag, 'k')
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title(filename + " FFT")
    plt.xlim(0, 100)

    plt.tight_layout()
    plt.savefig(filename.replace(".csv", "_time_fft.png"))
    plt.close()


def plot_maf(filename, X):
    t, signal = read_csv(filename)
    sample_rate = get_sample_rate(t)

    filtered = moving_average(signal, X)

    freqs, fft_unfiltered = get_fft(signal, sample_rate)
    freqs, fft_filtered = get_fft(filtered, sample_rate)

    plt.figure(figsize=(10, 6))

    # time-domain comparison
    plt.subplot(2, 1, 1)
    plt.plot(t, signal, 'k', label="Unfiltered")
    plt.plot(t, filtered, 'r', label="Filtered")
    plt.xlabel("Time (s)")
    plt.ylabel("Signal")
    plt.title(filename + " Moving Average Filter, X = " + str(X))
    plt.legend()

    # FFT comparison
    plt.subplot(2, 1, 2)
    plt.plot(freqs, fft_unfiltered, 'k', label="Unfiltered FFT")
    plt.plot(freqs, fft_filtered, 'r', label="Filtered FFT")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title(filename + " FFT Comparison")
    plt.xlim(0, 100)
    plt.legend()

    plt.tight_layout()
    plt.savefig(filename.replace(".csv", "_MAF.png"))
    plt.close()


def plot_iir(filename, A, B):
    t, signal = read_csv(filename)
    sample_rate = get_sample_rate(t)

    filtered = iir_filter(signal, A, B)

    freqs, fft_unfiltered = get_fft(signal, sample_rate)
    freqs, fft_filtered = get_fft(filtered, sample_rate)

    plt.figure(figsize=(10, 6))

    # time-domain comparison
    plt.subplot(2, 1, 1)
    plt.plot(t, signal, 'k', label="Unfiltered")
    plt.plot(t, filtered, 'r', label="Filtered")
    plt.xlabel("Time (s)")
    plt.ylabel("Signal")
    plt.title(filename + f" IIR Filter, A = {A}, B = {B}")
    plt.legend()

    # FFT comparison
    plt.subplot(2, 1, 2)
    plt.plot(freqs, fft_unfiltered, 'k', label="Unfiltered FFT")
    plt.plot(freqs, fft_filtered, 'r', label="Filtered FFT")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title(filename + " FFT Comparison")
    plt.xlim(0, 100)
    plt.legend()

    plt.tight_layout()
    plt.savefig(filename.replace(".csv", "_IIR.png"))
    plt.close()


def plot_fir(filename, cutoff_hz, num_weights):
    t, signal = read_csv(filename)
    sample_rate = get_sample_rate(t)

    weights = make_fir_weights(cutoff_hz, sample_rate, num_weights)
    filtered = fir_filter(signal, weights)

    freqs, fft_unfiltered = get_fft(signal, sample_rate)
    freqs, fft_filtered = get_fft(filtered, sample_rate)

    plt.figure(figsize=(10, 6))

    # time-domain comparison
    plt.subplot(2, 1, 1)
    plt.plot(t, signal, 'k', label="Unfiltered")
    plt.plot(t, filtered, 'r', label="Filtered")
    plt.xlabel("Time (s)")
    plt.ylabel("Signal")
    plt.title(
        filename
        + f" FIR Low-pass, cutoff = {cutoff_hz} Hz, weights = {num_weights}, Hamming window"
    )
    plt.legend()

    # FFT comparison
    plt.subplot(2, 1, 2)
    plt.plot(freqs, fft_unfiltered, 'k', label="Unfiltered FFT")
    plt.plot(freqs, fft_filtered, 'r', label="Filtered FFT")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title(filename + " FFT Comparison")
    plt.xlim(0, 100)
    plt.legend()

    plt.tight_layout()
    plt.savefig(filename.replace(".csv", "_FIR.png"))
    plt.close()


# ============================================================
# Run all plots
# ============================================================

# Original signal and FFT plots
plot_time_fft("sigA.csv")
plot_time_fft("sigB.csv")
plot_time_fft("sigC.csv")
plot_time_fft("sigD.csv")

# Moving Average Filter plots
plot_maf("sigA.csv", 300)
plot_maf("sigB.csv", 50)
plot_maf("sigC.csv", 100)
plot_maf("sigD.csv", 20)

# IIR Filter plots
plot_iir("sigA.csv", 0.95, 0.05)
plot_iir("sigB.csv", 0.98, 0.02)
plot_iir("sigC.csv", 0.8, 0.2)
plot_iir("sigD.csv", 0.9, 0.1)

# FIR Filter plots
plot_fir("sigA.csv", 10, 101)
plot_fir("sigB.csv", 5, 101)
plot_fir("sigC.csv", 10, 101)
plot_fir("sigD.csv", 5, 51)

print("Done! All plots were saved.")
