import time
import board
import pwmio

pwm = pwmio.PWMOut(board.GP15, frequency=50, duty_cycle=0)

def pulse_us(us):
    duty = int(us / 20000 * 65535)
    pwm.duty_cycle = duty

def angle_to_us(angle):
    if angle < 20:
        angle = 20
    if angle > 160:
        angle = 160
    return 500 + (angle / 180) * 2000

while True:
    for angle in range(20, 161, 5):
        pulse_us(angle_to_us(angle))
        time.sleep(0.05)

    for angle in range(160, 19, -5):
        pulse_us(angle_to_us(angle))
        time.sleep(0.05)