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

#ifndef __OPEN_VIDEO_INTELLIGENCE_PERFORMANCEMEASURE_H__
#define __OPEN_VIDEO_INTELLIGENCE_PERFORMANCEMEASURE_H__

#include <string>
#include <map>
#include <chrono>
#include <memory>
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

namespace ovi {

class PerformanceMeasure
{
public:
	static PerformanceMeasure& instance() {	//ToDo. Singleton is not recommended.
		static PerformanceMeasure _instance;
		return _instance;
	}

	void start();
	void split(const std::string& tag);
	void stop();

private:
	PerformanceMeasure() = default;
	~PerformanceMeasure() = default;
	PerformanceMeasure(PerformanceMeasure& other) = delete;
	void operator=(const PerformanceMeasure&) = delete;

	void addRecord(const std::string& tag);

	std::map<std::chrono::steady_clock::time_point, std::string> _timeRecord;
};

} // ovi

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPEN_VIDEO_INTELLIGENCE_PERFORMANCEMEASURE_H__ */
