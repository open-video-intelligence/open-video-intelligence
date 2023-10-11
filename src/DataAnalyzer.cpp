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

#include "DataAnalyzer.h"
#include <cmath>

using namespace ovi;

void DataAnalyzer::setCorrectionValue(double framerate)
{
	_frameCount = ceil(framerate);
}

bool DataAnalyzer::canSkip(const std::vector<RawData>& inputData, size_t i, size_t* skip)
{
	size_t len = static_cast<size_t>(_frameCount);

	if (i + len >= inputData.size())
		len = inputData.size() - i;

	for (size_t j = 1; j < len; j++) {
		if (inputData[i + j].include) {
			*skip = j;
			return true;
		}
	}

	return false;
}

void DataAnalyzer::analyzeRawData(const std::vector<RawData>& inputData)
{
	bool before = false;
	double start = -1.0;	//Frame number starts 0.
	double duration = 1.0;
	std::vector<RawData> temp;

	for (size_t i = 0; i < inputData.size(); i++) {
		if (!before && inputData[i].include) {
			start = inputData[i].frameNumber;
			before = inputData[i].include;
			if (!inputData[i].detected.empty())
				temp.push_back(inputData[i]);
		} else if (before && inputData[i].include) {
			duration++;
			if (!inputData[i].detected.empty())
				temp.push_back(inputData[i]);
		} else if (before && !inputData[i].include) {
			size_t skip = 0;
			if (canSkip(inputData, i, &skip)) {
				// Include current frame
				duration += (skip + 1);
				i += skip;
			} else {
				_timeRange.push_back({ start, duration, sort(temp) });
				start = -1.0;
				duration = 1.0;
				before = false;
				temp.clear();
				temp.shrink_to_fit();
			}
		}
	}

	if (start >= 0.0)
		_timeRange.push_back({ start, duration, sort(temp) });
}

const std::vector<TimeRangeWithMetadata>& DataAnalyzer::result() const
{
	return _timeRange;
}

SortedCollection DataAnalyzer::sort(const std::vector<RawData>& input)
{
	if (input.empty())
		return {};

	SortedCollection tmp;

	for (const auto& data : input) {
		for (const auto& [ uid, details ] : data.detected) {
			tmp[uid].push_back({ data.frameNumber, details });
		}
	}

	return tmp;
}
