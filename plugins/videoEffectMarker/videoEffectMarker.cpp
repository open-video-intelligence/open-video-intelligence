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

#include "IPluginEffect.h"

using namespace ovi;

class EffectMarker : public IPluginEffect
{
public:
	EffectMarker() = default;
	~EffectMarker() = default;

	int setAttrs(const std::map<std::string, std::string>& attrs) override;
	const std::map<std::string, std::string>& effectInfo() const override;

private:
	bool valid(const std::map<std::string, std::string>& attrs, const std::string& key);

	std::map<std::string, std::string> _effect;
};

bool EffectMarker::valid(const std::map<std::string, std::string>& attrs, const std::string& key)
{
	return (attrs.find(key) != attrs.end());
}

int EffectMarker::setAttrs(const std::map<std::string, std::string>& attrs)
{
	if (!valid(attrs, "name"))
		return OVI_ERROR_INVALID_PARAMETER;

	_effect = attrs;

	return OVI_ERROR_NONE;
}

const std::map<std::string, std::string>& EffectMarker::effectInfo() const
{
	return _effect;
}

extern "C" class IPlugin *createPlugin(void)
{
	return new EffectMarker();
}

extern "C" void destroyPlugin(class IPlugin *plugin)
{
	delete plugin;
}

extern "C" const char *name()
{
	return "VideoEffectMarker";
}

extern "C" PluginType type()
{
	return PLUGIN_TYPE_VIDEO_EFFECT;
}

extern "C" void *supportFormat()
{
	static std::vector<int> formats {
		VIDEO_FORMAT_NONE
	};

	return &formats;
}

extern "C" MetaForm supportMetaForm()
{
	return METAFORM_NONE;
}

extern "C" const char *description()
{
	return "Define the video effects to be given to the specified position";
}

extern "C" void *attributeList()
{
	static std::vector<Attribute> attrs {
		{ "name", "string", "Mandatory. Effect name to set." },
		{ "{option}", "string", "Additional information required for the effect can be added as a 'key/value'." }
	};

	return &attrs;
}
