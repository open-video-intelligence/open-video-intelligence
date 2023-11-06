/*
* Copyright (c) 2023 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <chrono>
#include <thread>

#include "utBase.h"
#include "Session.h"
#include "ovi.h"

using namespace std::chrono_literals;

class SessionTest : public UtBase
{
protected:
	void SetUp(void) override {
		Start();
	}

	void TearDown(void) override {
		End();
	}

	std::string getDetectPlugin();
	std::string getRenderPlugin();

	void prepare();

	std::string _renderName;
	std::string _detectPlugin;

	Session _session;
	bool _invoked {};
};

static void __dump_plugin_foreach_cb(const char* func, const char* name, ovi_plugin_type_e type, const char* description, void* user_data)
{
	std::cout << "\t[" << func << "] is invoked!!\n";
	std::cout << "\t--------------------------------\n";
	std::cout << "\tName        : " << name << "\n";
	std::cout << "\tType        : " << type << "\n";
	std::cout << "\tDescription : " << description << "\n";
	std::cout << "\tUserData    : " << user_data << "\n";
	std::cout << "\t--------------------------------\n";
}

static bool __plugin_detect_foreach_cb(const char* name, ovi_plugin_type_e type, const char* description, void* user_data)
{
	__dump_plugin_foreach_cb(__func__, name, type, description, user_data);

	auto detector = static_cast<std::string*>(user_data);
	if (detector && strcmp(name, "AudioDetect") == 0)
		*detector = name;

	return true;
}

static bool __plugin_render_foreach_cb(const char* name, ovi_plugin_type_e type, const char* description, void* user_data)
{
	__dump_plugin_foreach_cb(__func__, name, type, description, user_data);

	auto render = static_cast<std::string*>(user_data);
	if (render && strcmp(name, "OTIORender") == 0)
		*render = name;

	return true;
}

static void __errorCb(void* handle, ovi_error_e error, void* user_data)
{
	std::cout << "__errorCb() is invoked" << std::endl;
}

static void __progressCb(void* handle, const char* progress, void* user_data)
{
	std::cout << "__progressCb() is invoked:" << progress << std::endl;

	if (!user_data)
		return;

	auto invoked = static_cast<bool*>(user_data);

	*invoked = true;
}

static void __state_changed_cb(void* handle, ovi_state_e previous, ovi_state_e current, void* user_data)
{
	std::cout << "__state_changed_cb() is invoked: previous:" << previous << " current:" << current << std::endl;

	if (!user_data)
		return;

	auto invoked = static_cast<bool*>(user_data);

	if (previous == OVI_STATE_IDLE && current == OVI_STATE_ANALYSIS)
		*invoked = true;
}

static void __render_complete_cb(void* handle, ovi_state_e previous, ovi_state_e current, void* user_data)
{
	std::cout << "__render_complete_cb() is invoked: previous:" << previous << " current:" << current << std::endl;

	if (!user_data)
		return;

	auto invoked = static_cast<bool*>(user_data);

	if (previous == OVI_STATE_RENDER && current == OVI_STATE_IDLE)
		*invoked = true;
}

std::string SessionTest::getDetectPlugin()
{
	if (!_detectPlugin.empty())
		return _detectPlugin;

	int ret = ovi_available_plugin_foreach(OVI_PLUGIN_TYPE_NONE, NULL, __plugin_detect_foreach_cb, &_detectPlugin);
	if (ret != OVI_ERROR_NONE) {
		std::cout << "failed to ovi_available_plugin_foreach()" << std::endl;
		return "";
	}

	return _detectPlugin;
}

std::string SessionTest::getRenderPlugin()
{
	if (!_renderName.empty())
		return _renderName;

	int ret = ovi_available_plugin_foreach(OVI_PLUGIN_TYPE_RENDER, NULL, __plugin_render_foreach_cb, &_renderName);
	if (ret != OVI_ERROR_NONE) {
		std::cout << "failed to ovi_available_plugin_foreach()" << std::endl;
		return "";
	}

	return _renderName;
}

void SessionTest::prepare()
{
	std::vector<std::string> request {
		_session.appendPlugin(getDetectPlugin()),
		OVI_OP_OR,
		_session.appendPlugin(getDetectPlugin())
	};
	_session.registerPlugin(request);
	_session.setMediaPath(getMediaPath());
	_session.setRender(_session.appendPlugin(getRenderPlugin()), "./result.otio");
}

TEST_F(SessionTest, setMediaPath_check)
{
	try {
		_session.setMediaPath(getMediaPath());
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, setMediaPath_check_invalid_parameter_exception)
{
	try {
		_session.setMediaPath({});
		EXPECT_TRUE(false);
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(SessionTest, setMediaPath_check_no_such_file_exception)
{
	try {
		_session.setMediaPath("nosuchfile.mp4");
		EXPECT_TRUE(false);
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_NO_SUCH_FILE);
	}
}

TEST_F(SessionTest, start_check)
{
	prepare();

	try {
		_session.start();
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, start_check_and_wait_seconds)
{
	prepare();

	try {
		_session.start();
		std::this_thread::sleep_for(2s);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, start_check_invalid_operation_exception)
{
	try {
		_session.start();
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_OPERATION);
	}
}

TEST_F(SessionTest, stop_check)
{
	prepare();

	try {
		_session.start();
		_session.stop();
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, stop_check_after_wait_seconds)
{
	prepare();

	try {
		_session.start();
		std::this_thread::sleep_for(100ms);
		_session.stop();
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, stop_check_invalid_state_exception)
{
	try {
		_session.stop();
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_STATE);
	}
}

TEST_F(SessionTest, destroy_check_immediately)
{
	prepare();

	try {
		_session.start();
		_session.destroy();
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, destroy_check)
{
	prepare();

	try {
		_session.setStateChangedCb(__render_complete_cb, &_invoked);
		_session.setProgressCb(__progressCb, nullptr);
		_session.start();

		while (!_invoked)
			std::this_thread::sleep_for(2s);

		_session.destroy();
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, destroy_check_invalid_operation_exception)
{
	try {
		_session.destroy();
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_OPERATION);
	}
}


TEST_F(SessionTest, setRender_check)
{
	try {
		_session.setRender(_session.appendPlugin(getRenderPlugin()), "./result.otio");
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, setRender_check_invalid_parameter_exception)
{
	const std::vector<std::tuple<std::string, std::string>> params {
		// invalid uid
		{ "", "./result.otio" },
		{ "testUid", "./result.otio" },
		{ _session.appendPlugin(getDetectPlugin()), "./result.otio" },
		// invalid path
		{ _session.appendPlugin(getRenderPlugin()), "" },
	};

	for (const auto& [uid, path] : params) {
		try {
			_session.setRender(uid, path);
			EXPECT_TRUE(false);
		} catch (const Exception& e) {
			std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
			EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
		}
	}
}

TEST_F(SessionTest, appendPlugin_check)
{
	EXPECT_NE(_session.appendPlugin(getDetectPlugin()), "");
}

TEST_F(SessionTest, appendPlugin_check_invalid_parameter_exception)
{
	try {
		_session.appendPlugin("");
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(SessionTest, setPluginAttrs_check)
{
	std::map<std::string, std::string> attrs {
		std::pair<std::string, std::string>("path", "testImage.jpg"),
		std::pair<std::string, std::string>("model", "haarcascade_frontalface_alt2.xml")
	};

	try {
		_session.setPluginAttrs(_session.appendPlugin(getDetectPlugin()), attrs);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, setPluginAttrs_check_invalid_parameter_exception)
{
	std::map<std::string, std::string> attrs {
		std::pair<std::string, std::string>("path", "testImage.jpg"),
		std::pair<std::string, std::string>("model", "haarcascade_frontalface_alt2.xml")
	};

	try {
		_session.setPluginAttrs("", attrs);
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(SessionTest, registerPlugin_check)
{
	std::vector<std::string> request {
		_session.appendPlugin(getDetectPlugin()),
		OVI_OP_OR,
		_session.appendPlugin(getDetectPlugin())
	};

	try {
		_session.registerPlugin(request);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, registerPlugin_check_invalid_parameter_exception)
{
	try {
		_session.registerPlugin({});
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(SessionTest, setErrorCb_check)
{
	try {
		_session.setErrorCb(__errorCb, nullptr);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, setErrorCb_check_invalid_parameter_exception)
{
	try {
		_session.setErrorCb(nullptr, nullptr);
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(SessionTest, unsetErrorCb_check)
{
	try {
		_session.setErrorCb(__errorCb, nullptr);
		_session.unsetErrorCb();
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, unsetErrorCb_check_invalid_operation_exception)
{
	try {
		_session.unsetErrorCb();
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_OPERATION);
	}
}

TEST_F(SessionTest, setProgressCb_check)
{
	try {
		_session.setProgressCb(__progressCb, nullptr);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, setProgressCb_check_invoked)
{
	prepare();

	try {
		_session.setProgressCb(__progressCb, &_invoked);
		_session.start();

		while (!_invoked)
			std::this_thread::sleep_for(1s);

		EXPECT_TRUE(_invoked);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, setProgressCb_check_invalid_parameter_exception)
{
	try {
		_session.setProgressCb(nullptr, nullptr);
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(SessionTest, unsetProgressCb_check)
{
	try {
		_session.setProgressCb(__progressCb, nullptr);
		_session.unsetProgressCb();
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, unsetProgressCb_check_invalid_operation_exception)
{
	try {
		_session.unsetProgressCb();
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_OPERATION);
	}
}

TEST_F(SessionTest, setStateChangedCb_check)
{
	try {
		_session.setStateChangedCb(__state_changed_cb, nullptr);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, setStateChangedCb_check_invoked)
{
	prepare();

	try {
		_session.setStateChangedCb(__state_changed_cb, &_invoked);
		_session.start();

		while (!_invoked)
			std::this_thread::sleep_for(1s);

		EXPECT_TRUE(_invoked);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, setStateChangedCb_check_invalid_parameter_exception)
{
	try {
		_session.setStateChangedCb(nullptr, nullptr);
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(SessionTest, unsetStateChangedCb_check)
{
	try {
		_session.setStateChangedCb(__state_changed_cb, nullptr);
		_session.unsetStateChangedCb();
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(SessionTest, unsetStateChangedCb_check_invalid_operation_exception)
{
	try {
		_session.unsetStateChangedCb();
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_OPERATION);
	}
}
