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

import ovi
import time
import asyncio
import os
from unittest import TestCase, main
from typing import Final

OUTPUT: Final = './output.otio'
TESTMEDIA: Final = '../tests/unittest/resource/fd_hide.mp4'


class Lever:
    state: bool = False

    def __init__(self) -> None:
        pass

    def toggle(self) -> None:
        self.state = not self.state


lever = Lever()


def error_cb(error, userdata):
    pass


def prog_cb(msg, userdata):
    pass


def state_cb(pre, cur, userdata):
    if (pre == ovi.SessionState.OVI_STATE_RENDER and
            cur == ovi.SessionState.OVI_STATE_IDLE):
        lever.toggle()


class OviPluginListTest(TestCase):

    def testPluginListAll(self) -> None:
        plugins = ovi.plugin_list()
        self.assertTrue(len(plugins) > 0)

        for plugin in plugins:
            self.assertIsNotNone(plugin.name)
            self.assertIsNotNone(plugin.type)
            self.assertIsNotNone(plugin.description)

    def testPluginListType(self) -> None:
        plugins = ovi.plugin_list(
            ovi.PluginType.PLUGIN_TYPE_VIDEO_DETECT)

        self.assertTrue(len(plugins) > 0)

        for plugin in plugins:
            self.assertIsNotNone(plugin.name)
            self.assertIsNotNone(plugin.type)
            self.assertIsNotNone(plugin.description)

    def testPluginListTypeAndName(self) -> None:
        plugins = ovi.plugin_list(
            ovi.PluginType.PLUGIN_TYPE_VIDEO_DETECT, 'FaceDetect')

        self.assertTrue(len(plugins) == 1)

        for plugin in plugins:
            self.assertEqual(plugin.name, 'FaceDetect')
            self.assertEqual(plugin.type, ovi.PluginType.PLUGIN_TYPE_VIDEO_DETECT)
            self.assertEqual(plugin.description, 'Detecting faces')


class OviSessionCreateDestroyTest(TestCase):

    def testSessionInit(self) -> None:
        session = ovi.Session()
        self.assertIsNotNone(session)


class OviSessionMethodTest(TestCase):

    def setUp(self) -> None:
        self.session = ovi.Session()

    def testSessionAddPlugin(self) -> None:
        uid = self.session.add_plugin('FaceDetect')
        self.assertIsNotNone(uid)

    def testSessionAddPluginWithNoPlugin(self) -> None:
        uid = self.session.add_plugin('')
        self.assertIsNone(uid)

    def testSessionSetPluginAttribute(self) -> None:
        audio = self.session.add_plugin('AudioDetect')
        self.assertIsNotNone(audio)

        self.assertTrue(
            self.session.set_plugin_attribute(audio, {"threshold": "70"}))

    def testSessionSetPluginAttributeWithWrongUid(self) -> None:
        uid = 'WRONG.UID.1'

        self.assertFalse(
            self.session.set_plugin_attribute(uid, {"threshold": "70"}))

    def testSessionGetState(self) -> None:
        state = self.session.get_state()
        self.assertIs(state, ovi.SessionState.OVI_STATE_IDLE)

    def testSessionLinkSerivces(self) -> None:
        face = self.session.add_plugin('FaceDetect')
        self.assertIsNotNone(face)

        audio = self.session.add_plugin('AudioDetect')
        self.assertIsNotNone(audio)

        self.assertTrue(
            self.session.link_serivces([face, ovi.OP_OR, audio]))

    def testSessionSetErrorCallback(self) -> None:
        self.assertTrue(
            self.session.set_error_callback(error_cb, None))

    def testSessionSetProgressCallback(self) -> None:
        self.assertTrue(
            self.session.set_progress_callback(prog_cb, None))

    def testSessionSetStateChangedCallback(self) -> None:
        self.assertTrue(
            self.session.set_state_changed_callback(state_cb, None))

    def testSessionUnsetErrorCallback(self) -> None:
        self.assertTrue(
            self.session.set_error_callback(error_cb, None))

        self.assertTrue(self.session.unset_error_callback())

    def testSessionUnsetErrorCallbackWithNoSet(self) -> None:
        self.assertFalse(self.session.unset_error_callback())

    def testSessionUnsetProgressCallback(self) -> None:
        self.assertTrue(
            self.session.set_progress_callback(prog_cb, None))

        self.assertTrue(self.session.unset_progress_callback())

    def testSessionUnsetProgressCallbackWithNoSet(self) -> None:
        self.assertFalse(self.session.unset_progress_callback())

    def testSessionUnsetStateChangedCallback(self) -> None:
        self.assertTrue(
            self.session.set_state_changed_callback(state_cb, None))

        self.assertTrue(self.session.unset_state_changed_callback())

    def testSessionUnsetStateChangedCallbackWithNoSet(self) -> None:
        self.assertFalse(self.session.unset_state_changed_callback())

    def testSessionSetMediaPath(self) -> None:
        self.assertTrue(
            self.session.set_media_path(TESTMEDIA))

    def testSessionSetMediaPathWithNoFile(self) -> None:
        self.assertFalse(
            self.session.set_media_path('./No_file.mp4'))

    def testSessionSetRender(self) -> None:
        uid = self.session.add_plugin('OTIORender')
        self.assertIsNotNone(uid)

        self.assertTrue(
            self.session.set_render(uid, OUTPUT))

    def testSessionSetRenderWithWrongParam1(self) -> None:
        uid = 'WRONG.UID.1'

        self.assertFalse(
            self.session.set_render(uid, OUTPUT))

    def testSessionSetRenderWithWrongParam2(self) -> None:
        uid = self.session.add_plugin('OTIORender')
        self.assertIsNotNone(uid)

        self.assertFalse(
            self.session.set_render(uid, ''))

    def testSessionSetSkipVideoFrames(self) -> None:
        self.assertTrue(self.session.set_skip_video_frames(2))


async def watcher():
    while lever.state is False:
        time.sleep(1)


class OviSessionStartStopTest(TestCase):

    def setUp(self) -> None:
        self.session = ovi.Session()
        face = self.session.add_plugin('FaceDetect')
        self.assertIsNotNone(face)

        audio = self.session.add_plugin('AudioDetect')
        self.assertIsNotNone(audio)

        render = self.session.add_plugin('OTIORender')
        self.assertIsNotNone(render)

        self.assertTrue(
            self.session.set_plugin_attribute(audio, {"threshold": "70"}))

        self.assertTrue(
            self.session.link_serivces([face, ovi.OP_OR, audio]))

        self.assertTrue(
            self.session.set_state_changed_callback(state_cb, None))

        self.assertTrue(
            self.session.set_media_path(TESTMEDIA))

        self.assertTrue(
            self.session.set_render(render, OUTPUT))

    def tearDown(self) -> None:
        if os.path.isfile(OUTPUT):
            os.remove(OUTPUT)

    def testSessionStart(self) -> None:
        self.assertTrue(self.session.start())
        loop = asyncio.get_event_loop()
        loop.run_until_complete(watcher())

    def testSessionStartAndStop(self) -> None:
        self.assertTrue(self.session.start())
        time.sleep(1)
        self.assertTrue(self.session.stop())

    def testSessionStop(self) -> None:
        self.assertFalse(self.session.stop())


if __name__ == '__main__':
    main()
