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

import math
import numpy
import srcs.OVICommon as OVICommon
from typing import Dict, List, Final


class AudioDetector:

    def __init__(self) -> None:
        self.REF_PRES: Final = 0.00002
        self.threshold: float = 60.0
        self.inverse: bool = False

    def setThreshold(self, threshold: float) -> None:
        self.threshold = threshold

    def setInverse(self, inverse: bool) -> None:
        self.inverse = inverse

    def getResult(self, fltp: List[float]):
        db = self._toDecibel(self._calcRMS(fltp))
        res = (db > self.threshold)
        return (not res, db) if self.inverse else (res, db)

    def _toDecibel(self, energy: float) -> float:
        return 20.0 * math.log10(energy / self.REF_PRES) if energy != 0 else 0.0

    def _calcRMS(self, v: List[float]) -> float:
        return math.sqrt(sum(i**2 for i in v) / len(v))


ad = AudioDetector()


def setAttrs(attrs: Dict[str, str]) -> int:
    ad.setThreshold(float(attrs.get("threshold", "60.0")))
    ad.setInverse((attrs.get("inverse", "0") == "1"))
    return 0


def process(channels, samplerate, format_type, samples, frame):
    return ad.getResult(numpy.fromstring(frame, numpy.float32))


def pluginName():
    return "AudioDetectPy"


def pluginType():
    return OVICommon.Type.PLUGIN_TYPE_AUDIO_DETECT.value


def pluginFormat():
    return [OVICommon.AudioFormat.AUDIO_FORMAT_FLTP.value]


def pluginMetaForm():
    return OVICommon.MetaForm.METAFORM_DOUBLE.value


def pluginDescription():
    return "Detecting audio"


def pluginAttributeList():
    return [("threshold", "float", "db threshold"), ("inverse", "0 or 1", "silence detect")]
