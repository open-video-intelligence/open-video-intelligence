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

#ifndef __OPEN_VIDEO_INTELLIGENCE_OUTCOMECACHE_H__
#define __OPEN_VIDEO_INTELLIGENCE_OUTCOMECACHE_H__

#include "IPluginProcess.h"
#include <map>
#include <string>

namespace ovi {

using DetectedData = std::map<std::string, Details>;

class OutcomeCache
{
public:
	bool hit(const std::string& uid) const;
	void write(const std::string& uid, const Outcome& outcome);
	void setDetected(const std::string& uid);
	const DetectedData& detected() const;
	bool findMultiFrameResult();
	const Details& getMultiFrameResult();
	void setResultUid(const std::string& uid);
	const Outcome& result();
	void clear();

private:
	bool empty() const;
	void log(const std::string& uid, const Outcome& outcome) const;

	const Outcome _defaultOutcome = { true, {} };

	std::map<std::string, Outcome> _storage;
	DetectedData _detected;
	std::string _resultUid;
};

}

#endif // __OPEN_VIDEO_INTELLIGENCE_OUTCOMECACHE_H__
