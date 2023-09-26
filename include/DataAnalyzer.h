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

#ifndef __OPEN_VIDEO_INTELLIGENCE_DATA_ANALYZER_H__
#define __OPEN_VIDEO_INTELLIGENCE_DATA_ANALYZER_H__

#include "Accumulator.h"

namespace ovi {

//{ frame number, plugin's detected list }
struct Detected {
	double frameNumber {};
	Details list;
};

using SortedCollection = std::map<std::string, std::vector<Detected>>;

//{ Time range(start, duration), { Plugin UID, { frame number, plugin's detected list }}}
struct TimeRangeWithMetadata {
	TimeRange timeRange;
	SortedCollection collection;
};

class DataAnalyzer
{
public:
	DataAnalyzer() = default;
	~DataAnalyzer() = default;

	//You can set how much to ignore FALSE between TRUE.
	void setCorrectionValue(double framerate);
	void analyzeRawData(const std::vector<RawData>& inputData);
	const std::vector<TimeRangeWithMetadata>& result() const;

private:
	bool canSkip(const std::vector<RawData>& inputData, size_t i, size_t* skip);
	SortedCollection sort(const std::vector<RawData>& input);

	double _frameCount {};
	std::vector<RawData> _rawdata;
	std::vector<TimeRangeWithMetadata> _timeRange;
};

}

#endif // __OPEN_VIDEO_INTELLIGENCE_DATA_ANALYZER_H__
