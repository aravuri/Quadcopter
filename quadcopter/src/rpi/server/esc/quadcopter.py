import time

import pigpio

from rpi.server.esc.esc import launch_pi_gpiod, read_input


class ESCQuadcopter(object):

    def __init__(self, top_left_pin, top_right_pin, bottom_left_pin, bottom_right_pin,
                 min_value=700, max_value=2000):
        self._min_value = min_value
        self._max_value = max_value

        # top_left, top_right, bottom_left, bottom_right
        self._esc_pins = [top_left_pin, top_right_pin, bottom_left_pin, bottom_right_pin]

        launch_pi_gpiod()
        self.pi = pigpio.pi()
        self.set_value(0)
        time.sleep(4)

    def ramp_up_down(self, low, high, max_limit=100, delay=3.0):
        print("Ramping all motors up and then down... Press ENTER when ready")
        user_input = read_input()
        if user_input:
            return

        low, high = self.validate(low, high)
        if high - low >= max_limit:
            high = low + max_limit  # limit the ramp up limit

        value = low
        for delta in [10, -10]:
            while low <= value <= high:
                print("Setting {}...".format(value))
                self.set_value(value)
                time.sleep(delay)
                value += delta
        self.stop()

    def set_value(self, value):
        for esc_pin in self._esc_pins:
            self.pi.set_servo_pulsewidth(esc_pin, value)

    def validate(self, low, high):
        high = min(high, self._max_value)
        low = max(low, self._min_value)
        if low > high:
            print("low {} > high {}".format(low, high))
            raise Exception("Low > High")
        return low, high

    def control(self, speed=1500):
        print("Starting the motor. Make sure its calibrated and armed. If not, 'stop' and run with 'calibrate'")
        time.sleep(1)
        print("Choices: "
              "d to decrease speed; i to increase speed; "
              "dd to decrease speed by a lot; ii to increase speed by a lot")
        while True:
            self.set_value(speed)
            user_input = read_input()
            if user_input == "dd":
                speed -= 100
                print("speed = %d" % speed)
            elif user_input == "ii":
                speed += 100
                print("speed = %d" % speed)
            elif user_input == "i":
                speed += 10
                print("speed = %d" % speed)
            elif user_input == "d":
                speed -= 10
                print("speed = %d" % speed)
            elif user_input == "stop":
                self.stop()
                break
            else:
                print("Choose stop, d, i, dd or ii as options")

    def stop(self):
        self.set_value(0)
        self.pi.stop()
