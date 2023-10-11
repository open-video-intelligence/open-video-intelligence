/**
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

#include "PerformanceMeasure.h"
#include "Log.h"

using namespace ovi;

void PerformanceMeasure::addRecord(const std::string& tag)
{
	_timeRecord[std::chrono::steady_clock::now()] = tag;
}

void PerformanceMeasure::start()
{
	if (_timeRecord.size() != 0) {
		LOG_ERROR("Performance measurements already started!");
		return;
	}

	addRecord("PerformanceMeasureStart");
}

void PerformanceMeasure::split(const std::string& tag)
{
	if (_timeRecord.empty()) {
		LOG_ERROR("Performance measurements not started yet!");
		return;
	}

	addRecord(tag);
}

void PerformanceMeasure::stop()
{
	if (_timeRecord.empty()) {
		LOG_ERROR("Performance measurements not started!");
		return;
	}

	std::chrono::duration<double> prevTime {};
	std::chrono::steady_clock::time_point startTime {};
	std::chrono::steady_clock::time_point stopTime = std::chrono::steady_clock::now();

	for (const auto& [time, tag] : _timeRecord) {
		if (tag == "PerformanceMeasureStart") {
			startTime = time;
			continue;
		}

		auto overallTime = std::chrono::duration<double>(time - startTime);
		auto labTime = overallTime - prevTime;
		prevTime = overallTime;

		LOG_INFO("%s:%10.4fsec /%10.4fsec (labTime/overallTime)", tag.c_str(), labTime.count(), overallTime.count());
	}

	std::chrono::duration<double> totalElapsedTime = stopTime - startTime;
	LOG_INFO("%s:%10.4fsec", "total time", totalElapsedTime.count());

	_timeRecord.clear();
}
