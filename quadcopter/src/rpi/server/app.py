"""The main app module for the control server."""
from flask import Flask
from werkzeug.exceptions import default_exceptions

from common.app.config import Config, ServerResponse, ResponseData, error_handler
from rpi.server.routes.site.servo import servo


def create_app(**app_args):
    """Create Flask app with given configuration parameters."""
    app = Flask(__name__, template_folder="routes/templates")

    app.config.from_object(Config)
    app.config.update(**app_args)

    # Response class
    app.response_class = ServerResponse

    # Error status handlers: see https://coderwall.com/p/xq88zg/json-exception-handler-for-flask
    for code, value in default_exceptions.items():
        app.errorhandler(code)(error_handler)

    # See http://flask.pocoo.org/docs/0.12/blueprints/
    app.register_blueprint(servo)

    # Home page route
    @app.route("/")
    def home():
        """Home page route."""
        return ResponseData(200, data="Welcome to Ravuri SPL Systems!")

    return app
