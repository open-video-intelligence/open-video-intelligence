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

#include "Log.h"
#include "Configuration.h"
#include "Exception.h"

using namespace ovi;

namespace ovi::logger {
constexpr int DEFAULT_LOG_LEVEL = LOG_LEVEL_ALL;

static bool initialized = false;
static LogLevel logLevel = LOG_LEVEL_OFF;

void internal_error(const char* func, int line, const char* message);
}

void logger::internal_error(const char* func, int line, const char* message)
{
	std::cout << "logger::" << func << ":" << line << " " << message << std::endl;
}

void logger::init(void)
{
	init(Configuration::instance().get(CATEGORY_CORE, CORE_LOG_LEVEL, DEFAULT_LOG_LEVEL));
}

void logger::init(int level)
{
	if (initialized)
		return;
	logLevel = static_cast<LogLevel>(level);
	initialized = true;
}

void logger::init(const std::string& path)
{
	if (path.empty()) {
		internal_error(__func__, __LINE__, "Fail to Invalid Path");
		return;
	}

#if defined(__TIZEN__)
	// TODO: need to check file logger
	init(LOG_LEVEL_OFF);
#else
	init();
	stdLogger.setFileLogger(path);
#endif

	if (access(path.c_str(), W_OK) < 0) {
		if (errno == EACCES || errno == EPERM)
			internal_error(__func__, __LINE__, "Fail to open path: Permission Denied");
		else
			internal_error(__func__, __LINE__, "Fail to open path: Invalid Path");

		reset();
		return;
	}
}

void logger::reset()
{
	logLevel = LOG_LEVEL_OFF;
	initialized = false;

#if defined(__TIZEN__)
	// do nothing...
#else
	stdLogger.unsetFileLogger();
#endif
}

bool logger::validateLogLevel(LogLevel level)
{
	return (logLevel > level);
}
