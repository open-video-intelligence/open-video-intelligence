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

#include "Accumulator.h"
#include "Log.h"

using namespace ovi;

void Accumulator::append(double frameNumber, bool include, const DetectedData& detected)
{
	_rawdata.push_back({ frameNumber, include, detected });
}

void Accumulator::update(const Details& detected)
{
	int i = 0;

	for (auto& item : _rawdata) {
		LOG_INFO("[%f] : %d -> %d", item.frameNumber, item.include, std::get<bool>(detected[i]));
		item.include = std::get<bool>(detected[i++]);
	}
}

const std::vector<RawData>& Accumulator::accumulated()
{
	return _rawdata;
}
