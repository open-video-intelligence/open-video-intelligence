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

#ifndef __UT_BASE_H__
#define __UT_BASE_H__

#include <iostream>
#include <fstream>
#include <sstream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Log.h"
#include "Exception.h"

using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;

using namespace ovi;

class UtResource {
public:
	const std::string& getMediaPath() const { return _mediaPath; }
	const std::string& pluginName() const { return _pluginName; }
	const std::string& audioEffectPlugin() const { return _audioEffect; }

	static std::vector<char> readFile(std::string path)
	{
		std::ifstream fileRead(path);
		if (!fileRead.is_open()) {
			std::cout << "invalid path:" << path << std::endl;
			return {};
		}
		fileRead.seekg(0, fileRead.end);
		int length = fileRead.tellg();
		fileRead.seekg(0, fileRead.beg);

		std::vector<char> buffer(length);

		fileRead.read(buffer.data(), length);

		return buffer;
	}

private:
	const std::string _mediaPath = "fd_hide.mp4";
	const std::string _pluginName = "AudioDetect";
	const std::string _audioEffect = "AudioEffectMarker";
};

class UtLogger {
public:
	void Start(void) {
		std::string testSuiteName = ::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name();

		LOG_DEBUG("[Unittest] Start %s:%s", testSuiteName.c_str(),
			::testing::UnitTest::GetInstance()->current_test_info()->name());
	}

	void End(void) {
		LOG_DEBUG("[Unittest] End %s:%s",
			::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name(),
			::testing::UnitTest::GetInstance()->current_test_info()->name());
	}
};

class UtBase :
		public UtResource,
		public UtLogger,
		public ::testing::Test {
public:
	void SetUp(void) override {
		Start();
	}

	void TearDown(void) override {
		End();
	}
};

#endif	/*__UT_BASE_H__*/
