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

#ifndef __OPEN_VIDEO_INTELLIGENCE_ATTRIBUTE_VALIDATOR_H__
#define __OPEN_VIDEO_INTELLIGENCE_ATTRIBUTE_VALIDATOR_H__

#include <string>
#include <map>
#include <functional>
#include <vector>
#include <optional>

using AttrMap = std::map<std::string, std::string>;

struct AttributeSpec {
	std::string name;
	bool mandatory {};
	std::string spec;
	std::string defaultValue;
	std::optional<std::function<bool(std::string& s)>> validateFunc;
};

class AttributeValidator
{
public:
	AttributeValidator(const std::map<std::string, std::vector<AttributeSpec>>& attributesSpecMap);
	~AttributeValidator() = default;

	void validate(const AttrMap& attrs);

private:
	std::string parseEffectName(AttrMap& attrs);
	void validateEffectAttributes(AttrMap& attrs, const std::string& effectName);
	void checkUnhandled(const AttrMap& attrs, const std::string& effectName) const;

	std::map<std::string, std::vector<AttributeSpec>> _attributesSpecMap {};
};

#endif // __OPEN_VIDEO_INTELLIGENCE_ATTRIBUTE_VALIDATOR_H__
