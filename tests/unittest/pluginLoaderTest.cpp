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

#include "utBase.h"
#include "PluginLoader.h"


TEST(PluginManagerTest, getPluginAttrs_test)
{
	try {
		auto& attrs = PluginLoader::instance().getPluginAttrs("AudioDetect");
		EXPECT_FALSE(attrs.empty());

		for (auto& attrInfo : attrs) {
			EXPECT_NE(nullptr, attrInfo.key.c_str());
			EXPECT_NE(nullptr, attrInfo.type.c_str());
			EXPECT_NE(nullptr, attrInfo.description.c_str());
		}
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST(PluginManagerTest, getPluginList_test)
{
	auto& pluginList = PluginLoader::instance().getAvailablePluginList();

	for (auto& info : pluginList) {
		EXPECT_NE(nullptr, info.name.c_str());
		EXPECT_NE(PLUGIN_TYPE_NONE, info.type);
		EXPECT_NE(VIDEO_FORMAT_MAX, info.formats[0]);
		EXPECT_NE(nullptr, info.description.c_str());
	}
}

TEST(PluginManagerTest, createPlugin_test)
{
	try {
		Plugin testPlugin = PluginLoader::instance().load("AudioDetect");
		PluginLoader::instance().unload(testPlugin);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}
