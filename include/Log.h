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

#ifndef __OPEN_VIDEO_INTELLIGENCE_LOG_H__
#define __OPEN_VIDEO_INTELLIGENCE_LOG_H__

#include <string>

#ifdef TAG_NAME
#undef TAG_NAME
#endif
#define TAG_NAME "OVI"

namespace ovi::logger {

typedef enum {
	LOG_LEVEL_ALL = 0,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_OFF,
} LogLevel;

static LogLevel logLevel = LOG_LEVEL_ALL;

void init(void);
void init(int level);
void init(const std::string& path);

} // ovi::logger

#if defined(__TIZEN__)
#include <dlog.h>

#define LOG_DEBUG(...) do { \
	if (ovi::logger::logLevel > ovi::logger::LOG_LEVEL_DEBUG) \
		break; \
	dlog_print (DLOG_DEBUG, TAG_NAME, __VA_ARGS__); \
} while(0)

#define LOG_INFO(...) do { \
	if (ovi::logger::logLevel > ovi::logger::LOG_LEVEL_INFO) \
		break; \
	dlog_print (DLOG_INFO, TAG_NAME, __VA_ARGS__) \
} while(0)

#define LOG_WARN(...) do { \
	if (ovi::logger::logLevel > ovi::logger::LOG_LEVEL_WARNING) \
		break; \
	dlog_print (DLOG_WARN, TAG_NAME, __VA_ARGS__) \
} while(0)

#define LOG_ERROR(...) do { \
	if (ovi::logger::logLevel > ovi::logger::LOG_LEVEL_ERROR) \
		break; \
	dlog_print (DLOG_ERROR, TAG_NAME, __VA_ARGS__) \
} while(0)

#define LOG_ENTER() LOG_DEBUG("Enter")
#define LOG_LEAVE() LOG_DEBUG("Leave")

#else
// linux
#include "StdLog.h"

static ovi::StdLog stdLogger;

#define LOG_DEBUG(...) do { \
	if (ovi::logger::logLevel > ovi::logger::LOG_LEVEL_DEBUG) \
		break; \
	stdLogger.print(ovi::logger::LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, __VA_ARGS__); \
} while(0)

#define LOG_INFO(...) do { \
	if (ovi::logger::logLevel > ovi::logger::LOG_LEVEL_INFO) \
		break; \
	stdLogger.print(ovi::logger::LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, __VA_ARGS__); \
} while(0)

#define LOG_WARN(...) do { \
	if (ovi::logger::logLevel > ovi::logger::LOG_LEVEL_WARNING); \
		break; \
	stdLogger.print(ovi::logger::LOG_LEVEL_WARNING, __FILE__, __func__, __LINE__, __VA_ARGS__); \
} while(0)

#define LOG_ERROR(...) do { \
	if (ovi::logger::logLevel > ovi::logger::LOG_LEVEL_ERROR) \
		break; \
	stdLogger.print(ovi::logger::LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, __VA_ARGS__); \
} while(0)

#define LOG_ENTER() LOG_DEBUG("Enter")
#define LOG_LEAVE() LOG_DEBUG("Leave")
#endif

#endif // __OPEN_VIDEO_INTELLIGENCE_LOG_H__
