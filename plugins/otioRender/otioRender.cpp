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

#include "IPluginRender.h"

using namespace ovi;

class OtioRender : public IPluginRender
{
public:
	OtioRender() = default;
	~OtioRender() = default;

	int setAttrs(const std::map<std::string, std::string>& attrs) override;
	void render(timelineRetainer otioTimeline) override;
	MetaForm effectMetaForm(const std::string& effectName) override;

private:
	bool valid(const std::map<std::string, std::string>& attrs, const std::string& key);

	std::string _outputPath;
};

bool OtioRender::valid(const std::map<std::string, std::string>& attrs, const std::string& key)
{
	return (attrs.find(key) != attrs.end());
}

int OtioRender::setAttrs(const std::map<std::string, std::string>& attrs)
{
	if (!valid(attrs, "path"))
		return OVI_ERROR_INVALID_PARAMETER;

	_outputPath = attrs.at("path");

	return OVI_ERROR_NONE;
}

void OtioRender::render(timelineRetainer otioTimeline)
{
	otio::ErrorStatus err;

	if (_outputPath.empty())
		throw ovi::Exception(OVI_ERROR_INVALID_PARAMETER, "output path is empty");

	if (!otioTimeline->to_json_file(_outputPath, &err))
		throw ovi::Exception(OVI_ERROR_INVALID_OPERATION, std::string { "OtioRender ERROR: " } + err.details);
}

MetaForm OtioRender::effectMetaForm(const std::string& effectName)
{
	return METAFORM_ANY;
}

extern "C" class IPlugin *createPlugin(void)
{
	return new OtioRender();
}
extern "C" void destroyPlugin(class IPlugin *otio)
{
	delete otio;
}

extern "C" const char *name()
{
	return "OTIORender";
}

extern "C" PluginType type()
{
	return PLUGIN_TYPE_RENDER;
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
	return "This plugin saves the OTIO object to a given file.";
}

extern "C" void *attributeList()
{
	static std::vector<Attribute> attrs {
		{ "path", "string", "output file path" },
	};

	return &attrs;
}
