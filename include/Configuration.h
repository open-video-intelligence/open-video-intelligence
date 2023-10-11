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

#ifndef __OPEN_VIDEO_INTELLIGENCE_CONFIG_H__
#define __OPEN_VIDEO_INTELLIGENCE_CONFIG_H__

#ifdef OVI_ENABLE_INIPARSER
#include <iniparser/iniparser.h>
#endif
#include <iostream>
#include "Log.h"

namespace ovi {

/* categories */
const std::string CATEGORY_CORE = "core";

/* core items */
const std::string CORE_LOG_LEVEL = "log_level";
const std::string CORE_LOG_PATH = "log_path";

class Configuration
{
public:
	static Configuration& instance() {
		static Configuration _instance;
		return _instance;
	}

	int get(const std::string& category, const std::string& item, int defaultValue)
	{
		if (!validateParameters(category, item))
			return defaultValue;

#ifdef OVI_ENABLE_INIPARSER
		std::string key = category + ":" + item;
		int value = iniparser_getint(_dict, key.c_str(), defaultValue);
		log(key, value);

		return value;
#else
		return defaultValue;
#endif
	}

	std::string get(const std::string& category, const std::string& item, const std::string& defaultValue)
	{
		if (!validateParameters(category, item))
			return defaultValue;

#ifdef OVI_ENABLE_INIPARSER
		std::string key = category + ":" + item;
		const char* value = iniparser_getstring(_dict, key.c_str(), defaultValue.c_str());
		log(key, value);

		return std::string(value);
#else
		return defaultValue;
#endif
	}

private:
	Configuration() {
		std::cout << "[INI_FILE_PATH] " << std::string(INI_FILE_PATH) << std::endl;
#ifdef OVI_ENABLE_INIPARSER
		_dict = iniparser_load(INI_FILE_PATH);
#endif
	}
	~Configuration() {
#ifdef OVI_ENABLE_INIPARSER
		iniparser_freedict(_dict);
#endif
	}
	Configuration(Configuration& other) = delete;
	void operator=(const Configuration&) = delete;

	bool validateParameters(const std::string& category, const std::string& item) const {
		if (category.empty() || item.empty()) {
			LOG_WARN("invalid parameters");
			return false;
		}
		return true;
	}

	template<typename T1, typename T2>
	inline void log(const T1 &key, T2 value)
	{
		std::cout << "[key] " << key << ", [value] " << value << std::endl;
	}

#ifdef OVI_ENABLE_INIPARSER
	dictionary* _dict {};
#endif
};

} // ovi

#endif // __OPEN_VIDEO_INTELLIGENCE_CONFIG_H__