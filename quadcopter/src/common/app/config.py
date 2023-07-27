"""Application specific configuration."""
import os

from flask import Response, jsonify
from werkzeug.exceptions import HTTPException

from common.app.error_messages import ERROR_CODE_MESSAGES


class Config(object):
    """Configuration for Flask application."""

    DEBUG = os.getenv("DEBUG", "False") == "True"
    TESTING = os.getenv("TESTING", "False") == "True"


class ServerResponse(Response):
    """Server response properly formatted."""

    @classmethod
    def force_type(cls, response, environ=None):
        """Override of force_type method to coerce proper serialization of ResponseData."""
        if isinstance(response, ResponseData):
            status_code = response.status_code
            response = jsonify(response.to_dict())
            response.status_code = status_code
        return super().force_type(response, environ)


class ResponseData(object):
    """Special response data for all of our routes."""

    def __init__(self, status_code, error_code=None, data=None, metadata=None):
        """Constructor for ServerResponse class."""
        super().__init__()
        self.status_code = status_code
        self.data = data
        self.error_code = error_code
        self.message = ERROR_CODE_MESSAGES[self.error_code]
        self.metadata = metadata

    def to_dict(self):
        """Converts response to dict."""
        response = {"status_code": self.status_code}
        if self.error_code:
            response["error_code"] = self.error_code
            response["message"] = self.message
        if self.data:
            response["data"] = self.data
        if self.metadata:
            response["meta"] = self.metadata
        return response


def error_handler(exception):
    """Error handler for HTTP exceptions."""
    assert isinstance(exception, HTTPException)
    code = exception.code
    return ResponseData(code, error_code=code, data=exception.description)
