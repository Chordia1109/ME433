import serial
import glob
import math
import tkinter as tk
from collections import deque

# =========================
# Auto find Pico serial port
# =========================
ports = glob.glob("/dev/cu.usbmodem*")

if len(ports) == 0:
    raise RuntimeError("No Pico serial port found. Try: ls /dev/cu.usbmodem*")

SERIAL_PORT = ports[0]
BAUD_RATE = 115200

print("Using serial port:", SERIAL_PORT)

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.05)

# =========================
# Graphics settings
# =========================
WIDTH = 900
HEIGHT = 600

CENTER_X = 300
CENTER_Y = 300
PADDLE_LEN = 180

# 因为你的 force 大概只变 400–500，所以这里放大
FORCE_MAX = 1000   # force_raw = ±1000 时 bar 满格

angle_raw = 0
angle_deg = 0.0
force_raw = 0.0

force_history = deque(maxlen=80)

# =========================
# Tkinter window
# =========================
root = tk.Tk()
root.title("HW17 Graphics: AS5600 + Load Cell")

canvas = tk.Canvas(root, width=WIDTH, height=HEIGHT, bg="black")
canvas.pack()

def clamp(x, low, high):
    return max(low, min(high, x))

def read_serial():
    global angle_raw, angle_deg, force_raw

    try:
        line = ser.readline().decode(errors="ignore").strip()

        if not line:
            return

        # Skip header lines
        if "," not in line:
            return
        if line.startswith("angle"):
            return

        parts = line.split(",")

        if len(parts) >= 3:
            angle_raw = int(float(parts[0]))
            angle_deg = float(parts[1])
            force_raw = float(parts[2])
            force_history.append(force_raw)

    except Exception:
        pass

def draw():
    canvas.delete("all")

    # Read multiple lines per frame so it stays responsive
    for _ in range(5):
        read_serial()

    # =========================
    # Title
    # =========================
    canvas.create_text(
        30, 25,
        anchor="w",
        text="HW17: Pico reads AS5600 angle + HX711 force",
        fill="white",
        font=("Arial", 24, "bold")
    )

    # =========================
    # Draw paddle
    # =========================
    theta = math.radians(angle_deg)

    x2 = CENTER_X + PADDLE_LEN * math.cos(theta)
    y2 = CENTER_Y + PADDLE_LEN * math.sin(theta)

    canvas.create_oval(
        CENTER_X - 12, CENTER_Y - 12,
        CENTER_X + 12, CENTER_Y + 12,
        fill="gray"
    )

    canvas.create_line(
        CENTER_X, CENTER_Y, x2, y2,
        fill="cyan",
        width=10
    )

    canvas.create_oval(
        x2 - 18, y2 - 18,
        x2 + 18, y2 + 18,
        fill="yellow"
    )

    canvas.create_text(
        30, 70,
        anchor="w",
        text=f"angle_raw = {angle_raw}",
        fill="white",
        font=("Arial", 18)
    )

    canvas.create_text(
        30, 100,
        anchor="w",
        text=f"angle_deg = {angle_deg:.2f}",
        fill="white",
        font=("Arial", 18)
    )

    # =========================
    # Draw force bar
    # =========================
    bar_x = 560
    bar_y = 230
    bar_w = 260
    bar_h = 45

    canvas.create_text(
        bar_x, 170,
        anchor="w",
        text="Force display",
        fill="white",
        font=("Arial", 22, "bold")
    )

    canvas.create_text(
        bar_x, 205,
        anchor="w",
        text=f"force_raw = {force_raw:.0f}",
        fill="white",
        font=("Arial", 18)
    )

    # Center line
    center_bar_x = bar_x + bar_w / 2

    canvas.create_rectangle(
        bar_x, bar_y,
        bar_x + bar_w, bar_y + bar_h,
        outline="white",
        width=2
    )

    canvas.create_line(
        center_bar_x, bar_y - 10,
        center_bar_x, bar_y + bar_h + 10,
        fill="white",
        width=2
    )

    # Force bar: positive right, negative left
    force_clamped = clamp(force_raw, -FORCE_MAX, FORCE_MAX)
    force_px = (force_clamped / FORCE_MAX) * (bar_w / 2)

    if force_px >= 0:
        canvas.create_rectangle(
            center_bar_x, bar_y,
            center_bar_x + force_px, bar_y + bar_h,
            fill="red"
        )
    else:
        canvas.create_rectangle(
            center_bar_x + force_px, bar_y,
            center_bar_x, bar_y + bar_h,
            fill="blue"
        )

    canvas.create_text(
        bar_x, bar_y + 75,
        anchor="w",
        text=f"Force scale: ±{FORCE_MAX}",
        fill="gray",
        font=("Arial", 14)
    )

    # =========================
    # Draw force history graph
    # =========================
    graph_x = 520
    graph_y = 360
    graph_w = 320
    graph_h = 150

    canvas.create_rectangle(
        graph_x, graph_y,
        graph_x + graph_w, graph_y + graph_h,
        outline="white"
    )

    canvas.create_text(
        graph_x, graph_y - 25,
        anchor="w",
        text="Force history",
        fill="white",
        font=("Arial", 18)
    )

    # zero line
    zero_y = graph_y + graph_h / 2
    canvas.create_line(
        graph_x, zero_y,
        graph_x + graph_w, zero_y,
        fill="gray"
    )

    if len(force_history) > 1:
        points = []
        for i, f in enumerate(force_history):
            x = graph_x + i * graph_w / (len(force_history) - 1)
            f_clamped = clamp(f, -FORCE_MAX, FORCE_MAX)
            y = zero_y - (f_clamped / FORCE_MAX) * (graph_h / 2)
            points.append((x, y))

        for i in range(len(points) - 1):
            canvas.create_line(
                points[i][0], points[i][1],
                points[i+1][0], points[i+1][1],
                fill="lime",
                width=2
            )

    # =========================
    # Instructions
    # =========================
    canvas.create_text(
        30, 520,
        anchor="w",
        text="Turn paddle/magnet -> cyan paddle rotates",
        fill="white",
        font=("Arial", 16)
    )

    canvas.create_text(
        30, 550,
        anchor="w",
        text="Press load cell -> force bar and green graph change",
        fill="white",
        font=("Arial", 16)
    )

    root.after(30, draw)

draw()

try:
    root.mainloop()
finally:
    ser.close()
    