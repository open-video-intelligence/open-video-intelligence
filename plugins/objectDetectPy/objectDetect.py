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
from typing import Dict, Tuple, List
from dataclasses import asdict


class ObjectDetector:
    label = ''

    def __init__(self) -> None:
        self.model = os.path.join(OVICommon.MODEL_PATH, 'efficientdet_lite0.tflite')
        self.detector = vision.ObjectDetector.create_from_options(
            vision.ObjectDetectorOptions(
                base_options=python.BaseOptions(model_asset_path=self.model),
                score_threshold=0.5
            )
        )

    def setLabel(self, label) -> None:
        self.label = label

    def validLabel(self) -> bool:
        return 0 < len(self.label)

    def getResult(self, image):
        if not self.validLabel():
            return False, []
        d = self.detector.detect(
            mp.Image(image_format=mp.ImageFormat.SRGB, data=image)
        ).detections
        out = [
            tuple(asdict(b.bounding_box).values()) for b in d if
            b.categories[0].category_name == self.label
        ]
        return (len(out) > 0), out


od = ObjectDetector()


def setAttrs(attrs: Dict[str, str]) -> int:
    od.setLabel(attrs.get('label', ''))
    return 0


def process(width, height, frame, total_frame_num=0, fps=0.0) -> Tuple[bool, any]:
    detect, output = od.getResult(
        np.frombuffer(frame, dtype='B', count=int(height) * int(width) * 3).reshape(
            (int(height), int(width), 3)
        )
    )
    return (detect, output)


def pluginName() -> str:
    return "ObjectDetectPy"


def pluginType() -> int:
    return OVICommon.Type.PLUGIN_TYPE_VIDEO_DETECT.value


def pluginFormat() -> List[int]:
    return [OVICommon.VideoFormat.VIDEO_FORMAT_RGB24.value]


def pluginMetaForm() -> int:
    return OVICommon.MetaForm.METAFORM_RECT.value


def pluginDescription() -> str:
    return "Detecting objects"


def pluginAttributeList() -> List[Tuple[str, str, str]]:
    return [("label", "string", "Mandatory. Object label to find.")]
