# Copyright (c) 2023 Samsung Electronics Co., Ltd All Rights Reserved
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import mediapipe as mp
import numpy as np
from mediapipe.tasks import python
from mediapipe.tasks.python import vision
import srcs.OVICommon as OVICommon
from typing import Dict
from dataclasses import asdict


class FaceDetector:
    inverse = False

    def __init__(self):
        self.model = os.path.join(OVICommon.MODEL_PATH, 'blaze_face_short_range.tflite')
        self.detector = vision.FaceDetector.create_from_options(
            vision.FaceDetectorOptions(
                base_options=python.BaseOptions(model_asset_path=self.model),
                min_detection_confidence=0.6
            )
        )

    def setInverse(self, inverse):
        self.inverse = inverse

    def getFaces(self, image):
        d = self.detector.detect(mp.Image(image_format=mp.ImageFormat.SRGB, data=image))
        out = [tuple(asdict(b.bounding_box).values()) for b in d.detections]
        return (len(out) > 0), out


fd = FaceDetector()


def setAttrs(attrs: Dict[str, str]):
    fd.setInverse((attrs.get("inverse", "0") == "1"))
    return 0


def process(width, height, frame, total_frame_num=0, fps=0.0):
    detect, output = fd.getFaces(
        np.frombuffer(frame, dtype='B', count=int(height) * int(width) * 3).reshape(
            (int(height), int(width), 3)
        )
    )

    return (not detect, output) if fd.inverse else (detect, output)


def pluginName():
    return "FaceDetectPy"


def pluginType():
    return OVICommon.Type.PLUGIN_TYPE_VIDEO_DETECT.value


def pluginFormat():
    return [OVICommon.VideoFormat.VIDEO_FORMAT_RGB24.value]


def pluginMetaForm():
    return OVICommon.MetaForm.METAFORM_RECT.value


def pluginDescription():
    return "Detecting faces"


def pluginAttributeList():
    return [("inverse", "0 or 1", "faceless detect")]
