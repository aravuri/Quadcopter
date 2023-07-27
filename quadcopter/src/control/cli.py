"""Command line interface."""
import fire

from control.server.app import create_app
from control.server.camera.video import VideoFileSource, VideoDeviceSource, VideoDeviceWriterSource


class CLI(object):
    """Command line interface class."""

    @staticmethod
    def control_server(host="0.0.0.0", port=7777, debug=False):
        """Run the server to connect to the hardware."""
        app = create_app()
        app.run(host=host, port=port, debug=debug)

    @staticmethod
    def play_video(filename, delay=100):
        """Play a video file."""
        video_file = VideoFileSource(filename)
        frame = video_file.get_frame(delay)
        while frame is not None:
            video_file.show(frame, delay)
            frame = video_file.get_frame(delay)

    @staticmethod
    def stream_video(device_id=0, delay=100):
        """Stream video from a device."""
        video_stream = VideoDeviceSource(device_id)
        frame = video_stream.get_frame(delay)
        while frame is not None:
            video_stream.show(frame, delay)
            frame = video_stream.get_frame(delay)

    @staticmethod
    def write_streaming_video(filename, device_id=0, delay=100):
        """Stream video from a device and write it to a file."""
        video_stream = VideoDeviceWriterSource(device_id, filename)
        frame = video_stream.get_frame(delay)
        while frame is not None:
            video_stream.show(frame, delay)
            frame = video_stream.get_frame(delay)


###############################
if __name__ == "__main__":
    fire.Fire(CLI)
