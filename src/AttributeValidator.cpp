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

#include "AttributeValidator.h"
#include "Exception.h"
#include "Log.h"

using namespace ovi;

AttributeValidator::AttributeValidator(const std::map<std::string, std::vector<AttributeSpec>>& attrsInfo)
	: _attributesSpecMap(attrsInfo)
{
}

void AttributeValidator::validate(const AttrMap& attrs)
{
	auto copiedInputAttrs(attrs);

	std::string effectName = parseEffectName(copiedInputAttrs);
	validateEffectAttributes(copiedInputAttrs, effectName);
	checkUnhandled(copiedInputAttrs, effectName);
}

void AttributeValidator::checkUnhandled(const AttrMap& attrs, const std::string& effectName) const
{
	if (attrs.empty())
		return;

	for (const auto& [ key, value ] : attrs)
		LOG_ERROR("Unsupported effect attrs for %s [%s:%s]", effectName.c_str(), key.c_str(), value.c_str());

	throw ovi::Exception(OVI_ERROR_NOT_SUPPORTED_EFFECT_ATTR, "Unsupported effect attribute for " + effectName);
}

std::string AttributeValidator::parseEffectName(AttrMap& attrs)
{
	const auto& nameIter = attrs.find("name");
	if (nameIter == attrs.end())
		throw ovi::Exception(OVI_ERROR_INVALID_OPERATION, "No effect name");

	auto effectName = nameIter->second;
	if (_attributesSpecMap.find(effectName) == _attributesSpecMap.end())
		throw ovi::Exception(OVI_ERROR_NOT_SUPPORTED_EFFECT, "Unsupported effect:" + effectName);

	attrs.erase(nameIter);

	return effectName;
}

void AttributeValidator::validateEffectAttributes(AttrMap& attrs, const std::string& effectName)
{
	for (const auto& validator : _attributesSpecMap[effectName]) {
		const auto& attr = attrs.find(validator.name);
		if (attr == attrs.end()) {
			if (validator.mandatory)
				throw ovi::Exception(OVI_ERROR_INVALID_OPERATION, "Mandatory attribute is not set:" + validator.name);

			continue;
		}

		if (validator.validateFunc.has_value() &&
			!validator.validateFunc.value()(attr->second))
			throw ovi::Exception(OVI_ERROR_INVALID_EFFECT_ATTR_VALUE,
					"Invalid effect attribute value for " + effectName + ", " + attr->first + ":" + attr->second + ", " + validator.spec);

		attrs.erase(attr);
	}
}
