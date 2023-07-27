"""Request schemas for Raspberry Pi."""
from common.schemas.request import RequestSchema
from marshmallow import fields


class ServoRequestSchema(RequestSchema):
    """Servo motor request schema."""

    angle = fields.Float(required=False, load_from="angle", missing=10.0)
    pwm = fields.Int(required=False, missing=0, load_from="pwm")
    pin = fields.Int(required=False, missing=0, load_from="pin")
