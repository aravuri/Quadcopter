"""Adafruit PCA9685 servo board."""
import sys

import Adafruit_PCA9685

from rpi.server.servo.motor import ServoMotor


class AdafruitServoMotor(ServoMotor):
    """Servo motor like MG996R."""

    def __init__(self, channel=0, i2c_address=0x40, pwm_frequency=50, servo_min=115, servo_max=540,
                 min_angle=0.0, max_angle=180.0):
        """Construct the servo motor.

        Check channel and address using:
            sudo i2cdetect -y 1
        The defaults above (servo_min, servo_max) = (115, 540) are for one servo motor MG996R
        and (125, 480) for another servo motor MG996R.
        """
        super().__init__(Adafruit_PCA9685.PCA9685(address=i2c_address), pwm_frequency, servo_min, servo_max,
                         min_angle, max_angle)
        self._channel = channel
        self._i2c_address = i2c_address
        self._servo.set_pwm_freq(self._pwm_frequency)

        self._angle = 90
        self.set_pwm_by_angle(self._angle)

    def set_pwm(self, pulse, calibrating=False):
        """Set the PWM pulse with 12 bit resolution."""
        pulse = int(round(pulse))
        if not calibrating:
            pulse, angle = self._get_valid_servo_value(pulse)
            self._angle = angle
        self._servo.set_pwm(self._channel, 0, pulse)
        return self._angle, pulse

    def stop(self):
        """Stop the servo - used for cleaning up the motor."""
        pass


if __name__ == "__main__":
    servo_channel = 0
    if len(sys.argv) >= 2:
        servo_channel = int(sys.argv[1])
    servo_motor = AdafruitServoMotor(channel=servo_channel)
    low, high = servo_motor.calibrate()
    print("servo_min =", low, "servo_max =", high)
