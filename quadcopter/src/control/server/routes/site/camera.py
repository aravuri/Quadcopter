"""Camera control blueprints."""
from flask import Blueprint, g, request

from common.app.config import ResponseData
from control.server.camera import utils
from control.server.camera.video import VideoFileSource, VideoDeviceSource
from control.server.schemas.request import CameraRequestSchema

video = Blueprint("video", __name__, url_prefix="/video")
camera_objects = {}
video_devices = {}


@video.before_request
def before_request():
    """Pre-processing that occurs before each control request."""
    parsed_request, errors = CameraRequestSchema().load(request)
    if errors:
        return ResponseData(400, error_code=2500, data=errors)

    for key, value in parsed_request.items():
        setattr(g, key, value)


@video.route("/list/files", methods=["POST"])
def camera_list():
    """List all the video files opened.

    Sample request:
        curl -H "Content-Type: application/json" -X POST http://localhost:7777/video/list/files
    """
    data = [{"fileName": filename} for filename, video_file in camera_objects.items()]
    return ResponseData(200, data=data)


@video.route("/frame/file", methods=["POST"])
def play():
    """Get a frame from a video file.

    Sample request:
        curl -H "Content-Type: application/json" -X POST -d '{''
        "fileName": "20170924/drive.mov", "delay": 100}' http://localhost:7777/video/frame/file
    """
    if g.fileName not in camera_objects:
        video_file = VideoFileSource(g.fileName)
        camera_objects[g.fileName] = video_file
    else:
        video_file = camera_objects[g.g.fileName]

    frame = video_file.get_frame(g.delay)
    return ResponseData(200, data={"fileName": g.fileName,
                                   "delay": g.delay,
                                   "frame": frame})


@video.route("/frame/device/<int:device_id>", methods=["POST"])
def get_frame(device_id):
    """Get a frame from a video device.

    Sample request:
        curl -H "Content-Type: application/json" -X POST -d '{''
        "delay": 100}' http://localhost:7777/video/frame/device/0
    """
    if device_id in video_devices:
        video_device = video_devices[device_id]
    else:
        video_device = VideoDeviceSource(device_id)
        video_devices[device_id] = video_device

    frame = video_device.get_frame(g.delay)
    return ResponseData(200, data={"deviceId": device_id,
                                   "delay": g.delay,
                                   "frame": utils.encode(frame)})


@video.route("/frames/device/<int:device_id>", methods=["POST"])
def get_frames(device_id):
    """Get a frame from a video device.

    Sample request:
        curl -H "Content-Type: application/json" -X POST -d '{''
        "delay": 100, "duration": 5000}' http://localhost:7777/video/frames/device/0
    """
    if device_id in video_devices:
        video_device = video_devices[device_id]
    else:
        video_device = VideoDeviceSource(device_id)
        video_devices[device_id] = video_device

    frame = video_device.get_frames(g.duration, g.delay)
    return ResponseData(200, data={"deviceId": device_id,
                                   "delay": g.delay,
                                   "frame": utils.encode(frame)})


@video.route("/stop/device/<int:device_id>", methods=["POST"])
def stop_device(device_id):
    """Stop the device.

    Sample request:
        curl -H "Content-Type: application/json" -X POST http://localhost:7777/video/stop/device/0
    """
    if device_id in video_devices:
        video_device = video_devices[device_id]
        video_device.stop()
        del video_devices[device_id]

    return ResponseData(200, data={"status": "ok"})


@video.route("/stop/file", methods=["POST"])
def stop_file():
    """Stop the camera action.

    Sample request:
        curl -H "Content-Type: application/json" -X POST -d '{''
        "fileName": "20170924/drive.mov"}' http://localhost:7777/video/stop/file
    """
    video_file = camera_objects[g.fileName]
    video_file.stop()
    return ResponseData(200, data={"fileName": g.fileName})
