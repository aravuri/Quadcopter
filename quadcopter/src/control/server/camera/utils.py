import base64
import msgpack
import msgpack_numpy as m


def encode(frame):
    frame_enc = msgpack.packb(frame, default=m.encode)
    base64_enc = base64.b64encode(frame_enc).decode("utf-8")
    return base64_enc


def decode(base64_enc):
    base64_dec = base64.b64decode(base64_enc)
    frame = msgpack.unpackb(base64_dec, object_hook=m.decode)
    return frame
