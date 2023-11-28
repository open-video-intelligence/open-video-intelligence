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

import ctypes
import copy
from dataclasses import dataclass, field
from typing import List, Dict, Callable, Final
from enum import IntEnum

OP_AND: Final = '&'
OP_OR: Final = '|'
OP_COLON: Final = ':'
OP_UNCUT: Final = '~'


class PluginType(IntEnum):
    '''Plugin type
    '''
    PLUGIN_TYPE_NONE = 0,
    PLUGIN_TYPE_VIDEO_DETECT = 1,
    PLUGIN_TYPE_VIDEO_EFFECT = 2,
    PLUGIN_TYPE_AUDIO_DETECT = 3,
    PLUGIN_TYPE_AUDIO_EFFECT = 4,
    PLUGIN_TYPE_RENDER = 5,


class SessionState(IntEnum):
    '''State of session

    By default, it is in an idle state,
    and when analysis and render are completed,
    it returns to an idle state.
    '''
    OVI_STATE_IDLE = 0,
    OVI_STATE_ANALYSIS = 1,
    OVI_STATE_RENDER = 2,


@dataclass
class Attribute:
    '''Attribute information provided by the plugin

    Attributes:
        key (str): Attribute keyword
        type (str): Data type of attribute
        description (str): Description of attribute

    '''
    key: str = None
    type: str = None
    description: str = None


@dataclass
class Plugin:
    '''Plugin information

    Attributes:
        name (str): Plugin name
        type (PluginType): Plugin type
        description (str): Description of plugin
        attributes (List[Attribute]): List of attributes of plugin

    '''
    name: str = None
    type: PluginType = PluginType.PLUGIN_TYPE_NONE
    description: str = None
    attributes: List[Attribute] = field(default_factory=list)


def plugin_list(plugin_type=PluginType.PLUGIN_TYPE_NONE, name: str = None):
    '''Get a list of plugins.

    args:
        plugin_type (PluginType): Search with plugin type
        name (str): Search with plugin's name

    Returns:
        list[Plugin]: List of result
    '''
    libovi = ctypes.CDLL('/usr/local/lib/libovi.so', ctypes.RTLD_GLOBAL)

    @ctypes.CFUNCTYPE(
        ctypes.c_bool,
        ctypes.c_char_p,
        ctypes.c_int,
        ctypes.c_char_p,
        ctypes.c_void_p)
    def plugin_cb(name, plugin_type, desc, user):
        data = ctypes.cast(user, ctypes.POINTER(ctypes.py_object)).contents.value
        data.append(Plugin(name.decode(), PluginType(plugin_type), desc.decode()))

        return ctypes.c_bool(True)

    @ctypes.CFUNCTYPE(
        ctypes.c_bool,
        ctypes.c_char_p,
        ctypes.c_char_p,
        ctypes.c_char_p,
        ctypes.c_void_p)
    def attr_cb(key, key_type, desc, user):
        data = ctypes.cast(user, ctypes.POINTER(ctypes.py_object)).contents.value
        data.append(Attribute(key.decode(), key_type.decode(), desc.decode()))

        return ctypes.c_bool(True)

    pluginForeach = libovi.ovi_available_plugin_foreach
    pluginForeach.argtypes = [
        ctypes.c_int,
        ctypes.c_char_p,
        ctypes.CFUNCTYPE(
            ctypes.c_bool,
            ctypes.c_char_p,
            ctypes.c_int,
            ctypes.c_char_p,
            ctypes.c_void_p),
        ctypes.c_void_p
    ]
    pluginForeach.restype = ctypes.c_int

    encoded = name.encode() if name is not None else None
    plugins: List[Plugin] = []

    pluginForeach(
        ctypes.c_int(int(plugin_type)),
        encoded,
        plugin_cb,
        ctypes.cast(ctypes.pointer(ctypes.py_object(plugins)), ctypes.c_void_p))

    pluginAttrForeach = libovi.ovi_available_plugin_foreach_attribute
    pluginAttrForeach.argtypes = [
        ctypes.c_char_p,
        ctypes.CFUNCTYPE(
            ctypes.c_bool,
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_void_p),
        ctypes.c_void_p
    ]
    pluginAttrForeach.restype = ctypes.c_int

    attrs: List[Attribute] = []
    for i in plugins:
        pluginAttrForeach(
            i.name.encode(),
            attr_cb,
            ctypes.cast(ctypes.pointer(ctypes.py_object(attrs)), ctypes.c_void_p))
        i.attributes = copy.deepcopy(attrs)
        attrs.clear()

    return plugins


@ctypes.CFUNCTYPE(
    None,
    ctypes.c_void_p,
    ctypes.c_int,
    ctypes.c_int,
    ctypes.c_void_p)
def state_cb(handle, previous, current, user):
    data = ctypes.cast(user, ctypes.POINTER(ctypes.py_object)).contents.value
    data.func(SessionState(previous), SessionState(current), data.user)


@ctypes.CFUNCTYPE(
    None,
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_void_p)
def progress_cb(handle, progress, user):
    data = ctypes.cast(user, ctypes.POINTER(ctypes.py_object)).contents.value
    data.func(progress.decode(), data.user)


@ctypes.CFUNCTYPE(
    None,
    ctypes.c_void_p,
    ctypes.c_int,
    ctypes.c_void_p)
def err_cb(handle, error, user):
    data = ctypes.cast(user, ctypes.POINTER(ctypes.py_object)).contents.value
    data.func(error, data.user)


@dataclass
class CallbackData:
    func: any
    user: any


class Session:
    '''Session class
    '''

    def __init__(self) -> None:
        self.__lib = ctypes.CDLL('/usr/local/lib/libovi.so', ctypes.RTLD_GLOBAL)
        cfunc = self.__lib.ovi_session_create
        cfunc.argtypes = [ctypes.POINTER(ctypes.c_void_p)]
        cfunc.restype = ctypes.c_int

        self.__handle = ctypes.c_void_p(0)

        cfunc(ctypes.byref(self.__handle))

    def __del__(self) -> None:
        cfunc = self.__lib.ovi_session_destroy
        cfunc.argtypes = [ctypes.c_void_p]
        cfunc.restype = ctypes.c_int

        cfunc(self.__handle)

    def start(self) -> bool:
        '''Start session

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_start
        cfunc.argtypes = [ctypes.c_void_p]
        cfunc.restype = ctypes.c_int

        return (cfunc(self.__handle) == 0)

    def stop(self) -> bool:
        '''Stop session

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_stop
        cfunc.argtypes = [ctypes.c_void_p]
        cfunc.restype = ctypes.c_int

        return (cfunc(self.__handle) == 0)

    def get_state(self) -> SessionState:
        '''Get session state

        returns:
            SessionState: Current state of session
        '''
        cfunc = self.__lib.ovi_session_get_state
        cfunc.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_int)
        ]
        cfunc.restype = ctypes.c_int

        state = ctypes.c_int(-1)

        cfunc(self.__handle, ctypes.byref(state))

        return SessionState(state.value)

    def add_plugin(self, plugin_name: str) -> str:
        '''Add plugin to session

        args:
            plugin_name (str): Create given plugin

        returns:
            str: Returns UID of created plugin if successful, otherwise 'None'.
        '''
        cfunc = self.__lib.ovi_session_add_plugin
        cfunc.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_char_p)
        ]
        cfunc.restype = ctypes.c_int

        uid = ctypes.c_char_p(0)

        res = cfunc(self.__handle, plugin_name.encode(), ctypes.byref(uid))

        return uid.value.decode() if res == 0 else None

    def set_plugin_attribute(self, uid: str, attrs: Dict[str, str]) -> bool:
        '''Set plugin's attributes

        args:
            uid (str): Plugin UID
            attrs (Dict[str, str]): Attributes to set

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_set_plugin_attribute
        cfunc.restype = ctypes.c_int

        for key, value in attrs.items():
            res = cfunc(
                self.__handle,
                uid.encode(),
                key.encode(),
                value.encode(),
                None)

            if res != 0:
                return False

        return True

    def link_serivces(self, expr: List[str]) -> bool:
        '''Link plugins

        args:
            expr (List[str]): A list of plugins to link

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_link_plugins_with_list
        cfunc.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_char_p),
            ctypes.c_int
        ]
        cfunc.restype = ctypes.c_int

        toBytes = []
        for i in range(len(expr)):
            toBytes.append(bytes(expr[i], 'utf-8'))

        arr = (ctypes.c_char_p * (len(toBytes) + 1))()
        arr[:-1] = toBytes

        return (cfunc(self.__handle, arr, len(expr)) == 0)

    def set_media_path(self, path: str) -> bool:
        '''Set a path of a input media file

        args:
            path (str): Media file path

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_set_media_path
        cfunc.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        cfunc.restype = ctypes.c_int

        return (cfunc(self.__handle, path.encode()) == 0)

    def set_render(self, name: str, output_path: str) -> bool:
        '''Set a render plugin

        args:
            name (str): Name of render plugin
            output_path (str): File path to write the result

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_set_render
        cfunc.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_char_p
        ]
        cfunc.restype = ctypes.c_int

        return (cfunc(self.__handle, name.encode(), output_path.encode()) == 0)

    def set_skip_video_frames(self, skip: int) -> bool:
        '''Set the number of frame to skip

        args:
            skip (int): The number to skip

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_set_skip_video_frames
        cfunc.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
        cfunc.restype = ctypes.c_int

        return (cfunc(self.__handle, ctypes.c_size_t(skip)) == 0)

    def set_error_callback(self, func: Callable[[int, any], None], userdata: any) -> bool:
        '''Set an error callback on the session

        args:
            func (Callable[[int, any], None]): A function to be called if an error occurs
            userdata (any): User data to be passed to the callback function

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_set_error_cb
        cfunc.argtypes = [
            ctypes.c_void_p,
            ctypes.CFUNCTYPE(
                None,
                ctypes.c_void_p,
                ctypes.c_int,
                ctypes.c_void_p),
            ctypes.c_void_p
        ]
        cfunc.restype = ctypes.c_int

        cb = CallbackData(func, userdata)

        res = cfunc(
            self.__handle,
            err_cb,
            ctypes.cast(ctypes.pointer(ctypes.py_object(cb)), ctypes.c_void_p))

        return (res == 0)

    def set_progress_callback(self, func: Callable[[str, any], None], userdata: any) -> bool:
        '''Set a progress callback on the session

        args:
            func (Callable[[int, any], None]): A function to be called when progress is updated
            userdata (any): User data to be passed to the callback function

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_set_progress_cb
        cfunc.argtypes = [
            ctypes.c_void_p,
            ctypes.CFUNCTYPE(
                None,
                ctypes.c_void_p,
                ctypes.c_char_p,
                ctypes.c_void_p),
            ctypes.c_void_p
        ]
        cfunc.restype = ctypes.c_int

        cb = CallbackData(func, userdata)

        res = cfunc(
            self.__handle,
            progress_cb,
            ctypes.cast(ctypes.pointer(ctypes.py_object(cb)), ctypes.c_void_p))

        return (res == 0)

    def set_state_changed_callback(self, func: Callable[[SessionState, SessionState, any], bool],
                                   userdata: any) -> bool:
        '''Set a state changed callback on the session

        args:
            func (Callable[[int, any], None]): A function to be called when state is changed
            userdata (any): User data to be passed to the callback function

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_set_state_changed_cb
        cfunc.argtypes = [
            ctypes.c_void_p,
            ctypes.CFUNCTYPE(
                None,
                ctypes.c_void_p,
                ctypes.c_int,
                ctypes.c_int,
                ctypes.c_void_p),
            ctypes.c_void_p
        ]
        cfunc.restype = ctypes.c_int

        cb = CallbackData(func, userdata)

        res = cfunc(
            self.__handle,
            state_cb,
            ctypes.cast(ctypes.pointer(ctypes.py_object(cb)), ctypes.c_void_p))

        return (res == 0)

    def unset_error_callback(self) -> bool:
        '''unset an error callback from the session

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_unset_error_cb
        cfunc.argtypes = [ctypes.c_void_p]
        cfunc.restype = ctypes.c_int

        return (cfunc(self.__handle) == 0)

    def unset_progress_callback(self) -> bool:
        '''unset a progress callback from the session

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_unset_progress_cb
        cfunc.argtypes = [ctypes.c_void_p]
        cfunc.restype = ctypes.c_int

        return (cfunc(self.__handle) == 0)

    def unset_state_changed_callback(self) -> bool:
        '''unset a state changed callback from the session

        returns:
            bool: Returns 'True' if successful, otherwise 'False'.
        '''
        cfunc = self.__lib.ovi_session_unset_state_changed_cb
        cfunc.argtypes = [ctypes.c_void_p]
        cfunc.restype = ctypes.c_int

        return (cfunc(self.__handle) == 0)
