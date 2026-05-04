import tkinter as tk
import serial
import time
import random

SERIAL_PORT = "/dev/tty.usbmodem14301"
BAUD_RATE = 115200

WIDTH = 800
HEIGHT = 500

# Try to connect to Pico
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.01)
    time.sleep(2)
    print("Connected to Pico")
except Exception as e:
    ser = None
    print("Could not connect to Pico:", e)


root = tk.Tk()
root.title("HW10 Pico Button Flappy Game")

canvas = tk.Canvas(root, width=WIDTH, height=HEIGHT, bg="skyblue")
canvas.pack()

# Game variables
bird_x = 160
bird_y = 250
bird_vy = 0
gravity = 0.55
jump_strength = -8.5

pipe_width = 80
pipe_gap = 160
pipe_speed = 4
pipes = []

score = 0
game_over = False
started = False

last_button_value = 1
button_value = 1


# Draw objects
bird = canvas.create_oval(
    bird_x - 18, bird_y - 18,
    bird_x + 18, bird_y + 18,
    fill="yellow",
    outline="black",
    width=2
)

score_text = canvas.create_text(
    400, 40,
    text="Score: 0",
    fill="black",
    font=("Arial", 28, "bold")
)

instruction_text = canvas.create_text(
    400, 90,
    text="Press the Pico button to flap. Avoid the pipes!",
    fill="black",
    font=("Arial", 20)
)

status_text = canvas.create_text(
    400, 455,
    text="Press the button to start",
    fill="black",
    font=("Arial", 22, "bold")
)


def send_led(value):
    if ser is not None:
        try:
            if value == 1:
                ser.write(b"1")
            else:
                ser.write(b"0")
        except Exception:
            pass


def make_pipe():
    gap_y = random.randint(130, 350)

    top_pipe = canvas.create_rectangle(
        WIDTH, 0,
        WIDTH + pipe_width, gap_y - pipe_gap // 2,
        fill="green"
    )

    bottom_pipe = canvas.create_rectangle(
        WIDTH, gap_y + pipe_gap // 2,
        WIDTH + pipe_width, HEIGHT,
        fill="green"
    )

    pipes.append({
        "top": top_pipe,
        "bottom": bottom_pipe,
        "x": WIDTH,
        "passed": False
    })


def reset_game():
    global bird_y, bird_vy, pipes, score, game_over, started

    bird_y = 250
    bird_vy = 0
    score = 0
    game_over = False
    started = True

    for pipe in pipes:
        canvas.delete(pipe["top"])
        canvas.delete(pipe["bottom"])

    pipes = []
    make_pipe()

    canvas.coords(
        bird,
        bird_x - 18, bird_y - 18,
        bird_x + 18, bird_y + 18
    )

    canvas.itemconfig(score_text, text="Score: 0")
    canvas.itemconfig(status_text, text="Game running")
    send_led(0)


def flap():
    global bird_vy, started, game_over

    if game_over:
        reset_game()
        return

    if not started:
        reset_game()

    bird_vy = jump_strength
    send_led(1)


def read_pico_button():
    global button_value, last_button_value

    if ser is None:
        return False

    pressed_event = False

    try:
        # Read all available serial lines
        while ser.in_waiting > 0:
            line = ser.readline().decode("utf-8").strip()

            if line:
                parts = line.split(",")

                if len(parts) == 2 and parts[0] == "B":
                    button_value = int(parts[1])

                    # Detect new press: 1 -> 0
                    if last_button_value == 1 and button_value == 0:
                        pressed_event = True

                    last_button_value = button_value

    except Exception:
        pass

    return pressed_event


def check_collision():
    bird_box = canvas.coords(bird)

    # Hit top or bottom
    if bird_box[1] <= 0 or bird_box[3] >= HEIGHT:
        return True

    # Hit pipes
    for pipe in pipes:
        top_box = canvas.coords(pipe["top"])
        bottom_box = canvas.coords(pipe["bottom"])

        if boxes_overlap(bird_box, top_box) or boxes_overlap(bird_box, bottom_box):
            return True

    return False


def boxes_overlap(a, b):
    return not (
        a[2] < b[0] or
        a[0] > b[2] or
        a[3] < b[1] or
        a[1] > b[3]
    )


def update_game():
    global bird_y, bird_vy, score, game_over

    pressed = read_pico_button()

    # Also allow keyboard space for testing
    if pressed:
        flap()

    if started and not game_over:
        bird_vy += gravity
        bird_y += bird_vy

        canvas.coords(
            bird,
            bird_x - 18, bird_y - 18,
            bird_x + 18, bird_y + 18
        )

        # LED only flashes briefly during press
        if button_value == 1:
            send_led(0)

        # Move pipes
        for pipe in pipes:
            canvas.move(pipe["top"], -pipe_speed, 0)
            canvas.move(pipe["bottom"], -pipe_speed, 0)
            pipe["x"] -= pipe_speed

            # Score when bird passes pipe
            if not pipe["passed"] and pipe["x"] + pipe_width < bird_x:
                score += 1
                pipe["passed"] = True
                canvas.itemconfig(score_text, text=f"Score: {score}")

        # Remove old pipes
        if pipes and pipes[0]["x"] < -pipe_width:
            old_pipe = pipes.pop(0)
            canvas.delete(old_pipe["top"])
            canvas.delete(old_pipe["bottom"])

        # Add new pipe
        if len(pipes) == 0 or pipes[-1]["x"] < 450:
            make_pipe()

        # Collision
        if check_collision():
            game_over = True
            canvas.itemconfig(
                status_text,
                text="Game Over! Press button to restart"
            )
            send_led(1)

    root.after(25, update_game)


def keyboard_flap(event):
    flap()


root.bind("<space>", keyboard_flap)

update_game()
root.mainloop()