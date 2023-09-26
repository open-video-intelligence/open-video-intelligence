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

from enum import IntEnum
from typing import Final
import sys
import os


class Type(IntEnum):
    PLUGIN_TYPE_NONE = 0
    PLUGIN_TYPE_VIDEO_DETECT = 1
    PLUGIN_TYPE_VIDEO_EFFECT = 2
    PLUGIN_TYPE_AUDIO_DETECT = 3
    PLUGIN_TYPE_AUDIO_EFFECT = 4
    PLUGIN_TYPE_RENDER = 5


class VideoFormat(IntEnum):
    VIDEO_FORMAT_NONE = 0
    VIDEO_FORMAT_YUV420P = 1
    VIDEO_FORMAT_YUV422P = 2
    VIDEO_FORMAT_RGB24 = 3
    VIDEO_FORMAT_BGR24 = 4
    VIDEO_FORMAT_NV12 = 5
    VIDEO_FORMAT_NV21 = 6
    VIDEO_FORMAT_ARGB = 7
    VIDEO_FORMAT_RGBA = 8
    VIDEO_FORMAT_ABGR = 9
    VIDEO_FORMAT_BGRA = 10
    VIDEO_FORMAT_GRAY8 = 11


class AudioFormat(IntEnum):
    AUDIO_FORMAT_NONE = 0
    AUDIO_FORMAT_U8 = 1  # unsigned 8 bits
    AUDIO_FORMAT_S16 = 2  # signed 16 bits
    AUDIO_FORMAT_S32 = 3  # signed 32 bits
    AUDIO_FORMAT_FLT = 4  # float
    AUDIO_FORMAT_DBL = 5  # double
    AUDIO_FORMAT_S64 = 6  # signed 64 bits
    AUDIO_FORMAT_U8P = 7  # unsigned 8 bits, planar
    AUDIO_FORMAT_S16P = 8  # signed 16 bits, planar
    AUDIO_FORMAT_S32P = 9  # signed 32 bits, planar
    AUDIO_FORMAT_FLTP = 10  # float, planar
    AUDIO_FORMAT_DBLP = 11  # double, planar
    AUDIO_FORMAT_S64P = 12  # signed 64 bits, planar


class MetaForm(IntEnum):
    METAFORM_NONE = 0  # No Metadata
    METAFORM_ANY = 1  # Any
    METAFORM_DOUBLE = 10  # double
    METAFORM_RECT = 100  # Rectangle (double x, double y, double w, double h)
    METAFORM_RECT_STRING = 101  # Rectangle, string


LIB_PATH: Final = [p for p in sys.path if p.endswith('ovi/plugins')][0]
MODEL_PATH: Final = os.path.join(LIB_PATH, 'models')
UTIL_PATH: Final = os.path.join(LIB_PATH, 'srcs')
