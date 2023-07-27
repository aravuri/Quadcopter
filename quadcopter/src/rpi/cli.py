"""Command line interface."""
import fire

from rpi.server.app import create_app
from rpi.server.esc.esc import ESCCalibration
from rpi.server.esc.quadcopter import ESCQuadcopter
from rpi.server.servo import servo_board


class CLI(object):
    """Command line interface class."""

    @staticmethod
    def run_rpi_server(host="0.0.0.0", port=7778, debug=False):
        """Run the server to connect to the hardware."""
        app = create_app()
        app.run(host=host, port=port, debug=debug)

    @staticmethod
    def calibrate_servo_motor(channel):
        """Calibrate the servo motor when using Adafruit PCA9685 board."""
        servo_motor = servo_board.AdafruitServoMotor(channel=channel)
        low, high = servo_motor.calibrate()
        print("servo_min =", low, "servo_max =", high)

    @staticmethod
    def calibrate_esc(esc_pin, esc_min_value=700, esc_max_value=2000):
        """Calibrate the ESC."""
        esc = ESCCalibration(esc_pin, esc_min_value, esc_max_value)
        esc.run()

    @staticmethod
    def esc_quadcopter(low, high, top_left_pin=17, top_right_pin=18, bottom_left_pin=23, bottom_right_pin=24,
                       min_value=700, max_value=2000):
        """Good choice for low = 1500 and high = 1550."""
        esc_quad = ESCQuadcopter(top_left_pin, top_right_pin, bottom_left_pin, bottom_right_pin, min_value, max_value)
        try:
            esc_quad.ramp_up_down(low, high)
        except:
            esc_quad.stop()

    @staticmethod
    def manual_quadcopter(top_left_pin=17, top_right_pin=18, bottom_left_pin=23, bottom_right_pin=24,
                          start_value=1500, min_value=700, max_value=2000):
        """Manually increase or decrease the quadcopter motor speeds."""
        esc_quad = ESCQuadcopter(top_left_pin, top_right_pin, bottom_left_pin, bottom_right_pin, min_value, max_value)
        try:
            esc_quad.control(start_value)
        except:
            esc_quad.stop()


###############################
if __name__ == "__main__":
    fire.Fire(CLI)
