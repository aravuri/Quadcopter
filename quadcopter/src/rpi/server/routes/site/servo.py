"""Servo control blueprints."""
from flask import Blueprint, g, request

from common.app.config import ResponseData
from rpi.server.schemas.request import ServoRequestSchema
from rpi.server.servo.motor import ServoMotor

servo = Blueprint("servo", __name__, url_prefix="/servo")
servo_motors = {}


@servo.before_request
def before_request():
    """Pre-processing that occurs before each control request."""
    parsed_request, errors = ServoRequestSchema().load(request)
    if errors:
        return ResponseData(400, error_code=2000, data=errors)

    for key, value in parsed_request.items():
        setattr(g, key, value)


@servo.route("/<string:motor_name>/create", methods=["POST"])
def motor_create(motor_name):
    """Create a new servo motor.

    Sample request:
        curl -H "Content-Type: application/json" -X POST -d '{''
        "pin": 23}' http://localhost:7777/servo/left_pan/create
    """
    motor = None
    if g.pin != 0:
        motor = ServoMotor(g.pin)
    servo_motors[motor_name] = motor

    return ResponseData(200, data={"motor_name": motor_name,
                                   "pin": g.pin})


@servo.route("/list", methods=["POST"])
def motor_list():
    """Lists all servo motors that were created.

    Sample request:
        curl -H "Content-Type: application/json" -X POST http://localhost:7777/servo/list
    """
    data = [{"motor_name": name, "pin": motor.get_pin()} for name, motor in servo_motors.items()]
    return ResponseData(200, data=data)


@servo.route("/<string:motor_name>/turn", methods=["POST"])
def motor_turn(motor_name):
    """Control the motion of a given servo.

    Sample request:
        curl -H "Content-Type: application/json" -X POST -d '{''
        "angle": 1}' http://localhost:7777/servo/left_pan/turn
    """
    motor = servo_motors[motor_name]
    angle, pwm = g.angle, g.pwm
    if g.pwm != 0:
        motor.set_pwm(g.pwm)
    else:
        angle, pwm = motor.set_pwm_by_angle(g.angle)

    return ResponseData(200, data={"motor_name": motor_name,
                                   "angle": angle,
                                   "pwm": pwm})
