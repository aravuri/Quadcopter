"""Request schema."""
from marshmallow import fields

from common.schemas.request import RequestSchema


class CameraRequestSchema(RequestSchema):
    """Camera request schema."""

    fileName = fields.Str(required=False, load_from="fileName", missing="")
    delay = fields.Int(required=False, missing=100, load_from="delay")
    duration = fields.Int(required=False, missing=5000, load_from="duration")
