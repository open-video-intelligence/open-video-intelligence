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

#include <variant>

#include "Log.h"
#include "OutcomeCache.h"

using namespace ovi;

bool OutcomeCache::hit(const std::string& uid) const
{
	return (_storage.find(uid) != _storage.end());
}
void OutcomeCache::write(const std::string& uid, const Outcome& outcome)
{
	log(uid, outcome);
	_storage.insert_or_assign(uid, outcome);
	setResultUid(uid);
}

void OutcomeCache::setDetected(const std::string& uid)
{
	_detected.insert_or_assign(uid, result().list);
}

const DetectedData& OutcomeCache::detected() const
{
	return _detected;
}

bool OutcomeCache::findMultiFrameResult()
{
	for (auto it = _storage.begin(); it != _storage.end(); it++) {
		if (!it->second.list.empty() && std::holds_alternative<bool>(it->second.list[0]))
			return true;
	}

	return false;
}

const Details& OutcomeCache::getMultiFrameResult()
{
	for (auto it = _storage.begin(); it != _storage.end(); it++) {
		if (!it->second.list.empty() && std::holds_alternative<bool>(it->second.list[0]))
			return it->second.list;
	}

	throw ovi::Exception(OVI_ERROR_INVALID_OPERATION, "No items");
}

void OutcomeCache::setResultUid(const std::string& uid)
{
	_resultUid = uid;
}

const Outcome& OutcomeCache::result()
{
	if (empty())
		return _defaultOutcome;

	return _storage[_resultUid];
}

void OutcomeCache::clear()
{
	_storage.clear();
	_detected.clear();
	_resultUid.clear();
}

bool OutcomeCache::empty() const
{
	return (_storage.empty() || _resultUid.empty());
}

class VisitorLogItem {
public:
	explicit VisitorLogItem(const std::string& uid)
		: _uid(uid) {}

	void operator()(const OVIRect& rect) {
		LOG_DEBUG("[%s] rect: %f, %f, %f, %f",
			_uid.c_str(), rect.x, rect.y, rect.width, rect.height);
	}

	void operator()(const OVIRectTag& rectTag) {
		LOG_DEBUG("[%s] rectTag: %f, %f, %f, %f, %s",
			_uid.c_str(), rectTag.x, rectTag.y, rectTag.width, rectTag.height, rectTag.tag.c_str());
	}

	void operator()(const double& value) {
		LOG_DEBUG("[%s] double: %f", _uid.c_str(), value);
	}

	void operator()(const bool& value) {
		LOG_DEBUG("[%s] bool: %d", _uid.c_str(), value);
	}
private:
	std::string _uid;
};

void OutcomeCache::log(const std::string& uid, const Outcome& outcome) const
{
	LOG_DEBUG("[%s] detect: %d", uid.c_str(), outcome.detect);
	for (const auto& item : outcome.list) {
		std::visit(VisitorLogItem { uid }, item);
	}
}
