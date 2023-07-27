import os
import time

import pigpio
import psutil


def read_input():
    """Read the user input."""
    return input()


def launch_pi_gpiod():
    """Launch the GPIO library."""
    if check_pi_gpiod():
        return
    os.system("sudo pigpiod")
    time.sleep(1)


def check_pi_gpiod():
    """Check if pigpiod is already running or not."""
    pids = psutil.pids()
    for pid in pids:
        p = psutil.Process(pid)
        if p.name() == "pigpiod":
            return True
    return False


class ESCCalibration(object):
    """Calibrate an ESC."""

    def __init__(self, esc_pin, esc_min_value, esc_max_value):
        """Construct the ESC Calibration object."""
        self.esc_pin = esc_pin
        self.esc_min_value = esc_min_value
        self.esc_max_value = esc_max_value

        launch_pi_gpiod()
        self.pi = pigpio.pi()
        self.pi.set_servo_pulsewidth(esc_pin, 0)

    def run(self):
        """Run the calibration and control commands."""
        print("When launching for the first time, select calibrate from these options: "
              "calibrate, manual, control, arm or stop")
        user_input = read_input()
        if user_input == "manual":
            self.manual_drive()
        elif user_input == "calibrate":
            self.calibrate()
        elif user_input == "arm":
            self.arm()
        elif user_input == "control":
            self.control()
        elif user_input == "stop":
            self.stop()
        else:
            print("Invalid option...")

    def calibrate(self):
        """This is the auto calibration procedure of a normal ESC."""
        self.pi.set_servo_pulsewidth(self.esc_pin, 0)
        print("Disconnect the battery and press Enter")
        user_input = read_input()
        if user_input == "":
            self.pi.set_servo_pulsewidth(self.esc_pin, self.esc_max_value)
            print("Connect the battery NOW. You will hear two beeps. "
                  "Wait for a gradual falling tone, then press Enter.")
            user_input = read_input()
            if user_input == "":
                self.pi.set_servo_pulsewidth(self.esc_pin, self.esc_min_value)
                print("Special tone...")
                time.sleep(7)
                print("Wait for it...")
                time.sleep(5)
                print("Setting zero as value...")
                self.pi.set_servo_pulsewidth(self.esc_pin, 0)
                time.sleep(2)
                print("Arming ESC now...")
                self.pi.set_servo_pulsewidth(self.esc_pin, self.esc_min_value)
                time.sleep(1)
                print("Done calibrating...")
                self.control()

    def arm(self):
        """This is the arming procedure for an ESC."""
        print("Connect the battery and press Enter")
        user_input = read_input()
        if user_input == "":
            self.pi.set_servo_pulsewidth(self.esc_pin, 0)
            time.sleep(1)
            self.pi.set_servo_pulsewidth(self.esc_pin, self.esc_max_value)
            time.sleep(1)
            self.pi.set_servo_pulsewidth(self.esc_pin, self.esc_min_value)
            time.sleep(1)
            self.control()

    def manual_drive(self):
        """You will use this function to program your ESC if required."""
        print("Manual option: choose value between {} and {}".format(self.esc_min_value, self.esc_max_value))
        while True:
            user_input = read_input()
            if user_input == "stop":
                self.stop()
                break
            elif user_input == "control":
                self.control()
                break
            elif user_input == "arm":
                self.arm()
                break
            else:
                self.pi.set_servo_pulsewidth(self.esc_pin, user_input)

    def control(self, speed=1500):
        print("Starting the motor. Make sure its calibrated and armed. If not, 'stop' and run with 'calibrate'")
        time.sleep(1)
        print("Choices: "
              "d to decrease speed; i to increase speed; "
              "dd to decrease speed by a lot; ii to increase speed by a lot")
        while True:
            self.pi.set_servo_pulsewidth(self.esc_pin, speed)
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
            elif user_input == "manual":
                self.manual_drive()
                break
            elif user_input == "arm":
                self.arm()
                break
            else:
                print("Choose d, i, dd or ii as options")

    def stop(self):
        """This will stop every action your Pi is performing for ESC."""
        self.pi.set_servo_pulsewidth(self.esc_pin, 0)
        self.pi.stop()
