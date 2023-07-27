"""Control the servo motor."""
import time
from abc import ABC, abstractmethod


class ServoMotor(ABC):
    """Servo motor control."""

    def __init__(self, servo, pwm_frequency, servo_min, servo_max, min_angle=0.0, max_angle=180.0):
        """Setup the servo motor on a given pin id."""
        self._servo = servo
        self._pwm_frequency = pwm_frequency

        assert 0 <= servo_min < servo_max
        self._servo_min = servo_min
        self._servo_max = servo_max

        assert 0.0 <= min_angle < max_angle <= 180.0
        self._min_angle = min_angle
        self._max_angle = max_angle
        self._angle = 0

    @abstractmethod
    def set_pwm(self, pulse, calibrating=False):
        """Turn the servo motor by specifying a PWM signal directly. Returns (angle, PWM pulse)."""
        NotImplementedError("set_pwm() must be implemented...")

    @abstractmethod
    def stop(self):
        """Stop the servo - used for cleaning up the motor."""
        NotImplementedError("stop() must be implemented...")

    def calibrate(self):
        """Calibrate the servo motor."""
        user_input = input("Calibrating the servo motor. Press Ctrl+C to quit. Continue (y/n)? ")
        if user_input.lower() != "y":
            print("Aborting...")
            return None, None

        servo_max = self._sweep(64)
        servo_min = self._sweep(-64)
        return servo_min, servo_max

    def set_pwm_in_angle(self, angle):
        """Turn the servo motor to an absolute angle (degrees)."""
        angle = self._get_valid_servo_angle(angle)
        pulse = self._angle_to_pwm(angle)
        self._angle = angle
        self.set_pwm(pulse)
        return self._angle, pulse

    def set_pwm_by_angle(self, angle):
        """Turn the servo motor by an angle, positive or negative."""
        new_angle = self._angle + angle
        return self.set_pwm_in_angle(new_angle)

    def get_servo_min(self):
        """Get the minimum servo PWM value."""
        return self._servo_min

    def get_servo_max(self):
        """Get the maximum servo PWM value."""
        return self._servo_max

    def _angle_to_pwm(self, angle):
        """Map the angle to a PWM value."""
        return (angle - self._min_angle) * (self._servo_max - self._servo_min) / (self._max_angle - self._min_angle) \
            + self._servo_min

    def _pwm_to_angle(self, pulse):
        """Map the PWM value to an angle."""
        return (pulse - self._servo_min) * (self._max_angle - self._min_angle) / (self._servo_max - self._servo_min) \
            + self._min_angle

    def _get_valid_servo_value(self, pulse):
        """Convert the input servo value to a valid value."""
        angle = self._pwm_to_angle(pulse)
        if pulse >= self._servo_max:
            pulse = self._servo_max
            angle = self._max_angle
        if pulse <= self._servo_min:
            pulse = self._servo_min
            angle = self._min_angle
        return int(round(pulse)), angle

    def _get_valid_servo_angle(self, angle):
        """Convert the input servo angle to a valid value."""
        if angle < self._min_angle:
            angle = self._min_angle
        if angle > self._max_angle:
            angle = self._max_angle
        return angle

    def _sweep(self, delta, test_value=400):
        """If delta > 0, we are trying to find the max value and vice versa."""
        delta = int(delta)
        reset_value = test_value - delta
        while True:
            print("Trying new value...", test_value, delta)
            self.set_pwm(test_value, calibrating=True)
            time.sleep(1)

            user_input = input("Continue (y/n/end/specify PWM value/)? ")
            # increase or decrease the value to try based on user response on how the motor moved
            if user_input.lower() == "n":
                test_value -= delta
                delta //= 2
            if user_input.lower() == "end" or -1 <= delta <= 1:
                print("Best max servo value =", test_value)
                self.set_pwm(reset_value, calibrating=True)
                return test_value

            try:
                input_value = int(user_input)
            except ValueError:
                input_value = None
            if input_value:
                test_value = input_value
            else:
                test_value += delta

            print("Resetting back to...", reset_value, delta)
            self.set_pwm(reset_value, calibrating=True)
            time.sleep(1)

    def __del__(self):
        """Cleanup the servo motor when the object is deleted."""
        self.stop()
