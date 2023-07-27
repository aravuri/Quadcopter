"""Request schema."""
from marshmallow import Schema, pre_load


class RequestSchema(Schema):
    """Base Flask request parsing schema."""

    @pre_load
    def parse_request(self, request):
        """Splats all request data into a dictionary."""
        values = request.values.to_dict() or {}
        json = request.json or {}
        view_args = request.view_args or {}
        return {**values, **json, **view_args}
