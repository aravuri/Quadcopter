"""Control the servo motor."""
from RPIO import PWM

from rpi.server.servo.motor import ServoMotor


class RPIOServoMotor(ServoMotor):
    """Servo motor control."""

    def __init__(self, pwm_pin_id, pwm_frequency=50, servo_min=550, servo_max=2500, min_angle=0.0, max_angle=180.0):
        """Setup the servo motor on a given pin id."""
        super().__init__(PWM.Servo(), pwm_frequency, servo_min, servo_max, min_angle, max_angle)
        self._pwm_pin_id = pwm_pin_id

        self._angle = 90
        self.set_pwm_by_angle(self._angle)

    def get_pin(self):
        """Get the pin id used for this motor."""
        return self._pwm_pin_id

    def set_pwm(self, pulse, calibrating=False):
        """Set the PWM pulse in microseconds with a max value of 20000 us = 20 ms."""
        pulse = int(round(pulse))
        if not calibrating:
            pulse, angle = self._get_valid_servo_value(pulse)
            self._angle = angle
        self._servo.set_servo(self._pwm_pin_id, pulse)
        return self._angle, pulse

    def stop(self):
        """Stop the servo - used for cleaning up the motor."""
        self._servo.stop_servo(self._pwm_pin_id)
