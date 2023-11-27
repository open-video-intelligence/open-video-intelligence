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

#include <cstdarg>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
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
		std::filesystem::path p(file);

		// [Date Time][PID/TID][filename][function:line] message
		message += currentDateTime();
		message += stringFormat("[%d/%d]", getpid(), gettid());
		message += stringFormat("[%s]", LOG_LEVEL_STR[level]);
		message += stringFormat("[%s][%s:%d] ", p.filename().c_str(), caller, line);
		message += stringFormat(format, args ... );

		std::cout << COLOR_STR[level] << message << COLOR_STR[0] << std::endl;
	}

	void setFileLogger(const std::string& path)
	{
		try {
			std::filesystem::path fsPath = path;
			std::filesystem::create_directories(fsPath.parent_path());

			_fstream.open(fsPath, std::ios_base::app);
			if (!_fstream.fail())
				std::cout.rdbuf(_fstream.rdbuf());
		} catch (std::filesystem::filesystem_error const& e) {
			std::cout << e.what() << std::endl;
		}
	}

	void unsetFileLogger()
	{
		if (_fstream.is_open())
			_fstream.close();
	}

private:
	std::string stringFormat(const std::string& format, ...)
	{
		std::va_list args1;
		va_start(args1, format);

		std::va_list args2;
		va_copy(args2, args1);

		size_t size = std::vsnprintf(nullptr, 0, format.c_str(), args1);
		std::vector<char> buf(size + 1);
		va_end(args1);

		std::vsnprintf(buf.data(), buf.size(), format.c_str(), args2);
		va_end(args2);

		return std::string(buf.data(), size);
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

	std::fstream _fstream {};
};

}

#endif // __TIZEN__

#endif // __OPEN_VIDEO_INTELLIGENCE_STDLOG_H__
