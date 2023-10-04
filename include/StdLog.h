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

#ifndef __OPEN_VIDEO_INTELLIGENCE_STDLOG_H__
#define __OPEN_VIDEO_INTELLIGENCE_STDLOG_H__

#ifndef __TIZEN__

#include <unistd.h>

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace ovi {

class StdLog {
public:
	StdLog() = default;
	~StdLog() = default;

	template<typename ... Args>
	void print(int level, const char* file, const char* caller, int line, const std::string& format, Args ... args)
	{
		std::string message;

		// [Date Time][PID/TID][filename][function:line] message
		message += currentDateTime();
		message += stringFormat("[%d/%d]", getpid(), gettid());
		message += stringFormat("[%s]", LOG_LEVEL_STR[level]);
		if (file)
			message += stringFormat("[%s][%s:%d]", (std::strrchr(file, '/') ? std::strrchr(file, '/') + 1 : file), caller, line);
		else
			message += stringFormat("[%s][%s:%d]", file, caller, line);
		message += " ";
		message += stringFormat(format, args ... );

		std::cout << COLOR_STR[level] << message << COLOR_STR[0] << std::endl;
	}

private:
	template<typename ... Args>
	std::string stringFormat(const std::string& format, Args ... args)
	{
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0)
			throw std::runtime_error("Error during formatting.");

		auto size = static_cast<size_t>(size_s);
		std::unique_ptr<char[]> buf(new char[size]);
		std::snprintf(buf.get(), size, format.c_str(), args ...);

		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

	std::string currentDateTime()
	{
		time_t now = time(0);
		struct tm tstruct;
		char  buf[50];

		tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", &tstruct);

		return buf;
	}

	const std::vector<const char*> LOG_LEVEL_STR = {
		"none",
		"D", // debug
		"I", // info
		"W", // warning
		"E", // error
	};

	const std::vector<std::string> COLOR_STR = {
		"\33[0m", // reset
		"\33[0m", // if debug, default
		"\33[36m", // if info, cyan
		"\33[33m", // if warning, yellow
		"\33[31m", // if error, red
	};
};

}

#endif // __TIZEN__

#endif // __OPEN_VIDEO_INTELLIGENCE_STDLOG_H__