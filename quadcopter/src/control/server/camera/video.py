"""Camera video related functionality."""
import cv2
import numpy as np
import time
from abc import abstractmethod, ABC

from common.utils import file as file_utils


class VideoSource(ABC):

    def __init__(self):
        self._stop = False

    @abstractmethod
    def get_frame(self, delay=100):
        raise NotImplementedError("Get a video frame from this source.")

    @abstractmethod
    def get_frames(self, duration=5000, delay=100):
        raise NotImplementedError("Get a set of frames from this source for a given duration in milliseconds.")

    @abstractmethod
    def release(self):
        """Release all resources."""
        raise NotImplementedError("Release all resources.")

    @staticmethod
    def show(frames, delay=100):
        """Show the frames."""
        if len(frames.shape) == 3:
            frames = frames.reshape(1, *frames.shape)
        for frame in frames:
            cv2.waitKey(delay)
            cv2.imshow("Video", frame)

    def stop(self):
        """Stop using this source."""
        self._stop = True
        self.release()

    def __del__(self):
        """Delete all resources."""
        self.release()


class VideoFileSource(VideoSource):

    def __init__(self, video_file):
        """Construct a video source from file."""
        super().__init__()
        self._video_file = file_utils.make_dirs(video_file, file=True)
        self._video_capture = cv2.VideoCapture(self._video_file)

    def get_frame(self, delay=100):
        if self._stop:
            return None

        cv2.waitKey(delay)
        ret, frame = self._video_capture.read()
        return frame

    def get_frames(self, duration=5000, delay=100):
        """Get a set of frames for a given duration."""
        frames = []
        start = time.time()
        elapsed = 0.0
        while not self._stop and self._video_capture.isOpened() and elapsed <= duration / 1000.0:
            frame = self.get_frame(delay)
            if frame is None:
                break
            frames.append(frame)
            elapsed = time.time() - start
        return np.array(frames)

    def release(self):
        """Release all resources."""
        self._video_capture.release()
        cv2.destroyAllWindows()


class VideoDeviceSource(VideoSource):

    def __init__(self, device_id):
        """Construct the video source from a device."""
        super().__init__()
        self._device_id = device_id
        self._video_capture = cv2.VideoCapture(self._device_id)
        self.get_frames(duration=10, delay=100)

    def get_frame(self, delay=100):
        """Stream a single frame of video."""
        if self._stop:
            return None

        cv2.waitKey(delay)
        ret, frame = self._video_capture.read()
        return frame

    def get_frames(self, duration=5000, delay=100):
        """Get a set of frames for a given duration in milliseconds."""
        frames = []
        start = time.time()
        elapsed = 0.0
        while not self._stop and elapsed <= duration / 1000.0:
            frame = self.get_frame(delay)
            if frame is None:
                break
            frames.append(frame)
            elapsed = time.time() - start
        return np.array(frames)

    def release(self):
        """Release all resources for cleanup."""
        self._video_capture.release()
        cv2.destroyAllWindows()


class VideoDeviceWriterSource(VideoDeviceSource):

    def __init__(self, device_id, video_file, fps=10.0, resolution=(1920, 1080)):
        """Construct the camera writer."""
        super().__init__(device_id)
        self._video_file = file_utils.make_dirs(video_file, file=True)
        self._fps = fps
        self._resolution = resolution
        self._fourcc = cv2.VideoWriter_fourcc(*"XVID")
        self._video_writer = cv2.VideoWriter(self._video_file, self._fourcc, self._fps, self._resolution)

    def get_frame(self, delay=100):
        """Stream and write a single frame of video."""
        if self._stop:
            return None

        cv2.waitKey(delay)
        ret, frame = self._video_capture.read()
        if ret:
            self._video_writer.write(frame)

        return frame

    def release(self):
        """Release all resources."""
        super(VideoDeviceWriterSource, self).release()
        self._video_writer.release()
