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

#include "ffmpegEffect.h"

using namespace ovi;

class FFmpegRender : public IPluginRender
{
public:
	FFmpegRender();
	~FFmpegRender() = default;

	int setAttrs(const std::map<std::string, std::string>& attrs) override;
	void validateEffectAttrs(const std::map<std::string, std::string>& attrs) override;
	void render(timelineRetainer otioTimeline) override;
	MetaForm effectMetaForm(const std::string& effectName) override;
	static std::string makeDescription();

private:
	void cut();
	bool valid(const std::map<std::string, std::string>& attrs, const std::string& key);
	std::string toTimeString(double time);

	timelineRetainer _otioTimeline;
	std::string _outputPath;
	std::unique_ptr<FFmpegEffect> _effect;
	std::unique_ptr<AttributeValidator>_attrsValidator;
	std::string _effectOutputPath;

};

FFmpegRender::FFmpegRender()
	: _attrsValidator (std::make_unique<AttributeValidator>(FFmpegEffectSpec::attributes()))
{
}

bool FFmpegRender::valid(const std::map<std::string, std::string>& attrs, const std::string& key)
{
	return (attrs.find(key) != attrs.end());
}

int FFmpegRender::setAttrs(const std::map<std::string, std::string>& attrs)
{
	if (!valid(attrs, "path"))
		return OVI_ERROR_INVALID_PARAMETER;

	_outputPath = attrs.at("path");
	std::cout << "outputPath:" << _outputPath << std::endl;

	return OVI_ERROR_NONE;
}

void FFmpegRender::validateEffectAttrs(const std::map<std::string, std::string>& attrs)
{
	return _attrsValidator->validate(attrs);
}

std::string FFmpegRender::toTimeString(double time)
{
	char timestring[20];

	int ms = static_cast<int>(time * 1000.);
	int h = ms / (1000 * 60 * 60);
	ms -= h * (1000 * 60 * 60);

	int m = ms / (1000 * 60);
	ms -= m * (1000 * 60);

	int s = ms / 1000;
	ms -= s * 1000;

	snprintf(timestring, 20, "%.2d:%.2d:%.2d.%.3d", h%100, m%100, s%100, ms%1000);

	return timestring;
}

void FFmpegRender::cut()
{
	const std::vector<clipRetainer> clips = _otioTimeline->find_clips();
	int len = 0;
	char tmp[30];

	if (clips.empty()) {
		std::cout << "No clips" << std::endl;
		return;
	}

	for (const auto& clip : clips) {
		auto end = clip->trimmed_range().duration() + clip->trimmed_range().start_time();
		auto ex = dynamic_cast<otio::ExternalReference*>(clip->media_reference());
		assert(ex);

		std::string cmd;
		cmd += "/usr/bin/ffmpeg -y -ss ";
		cmd += toTimeString(clip->trimmed_range().start_time().to_seconds());
		cmd += " -to ";
		cmd += toTimeString(end.to_seconds());
		cmd += " -i ";
		cmd += _effectOutputPath.empty()? ex->target_url() : _effectOutputPath;
		cmd += " -c copy ";
		len++;
		snprintf(tmp, 30, "tmp_%.3d.mp4", len);
		cmd += tmp;

		std::cout << "ffmpegRender cmd:" << cmd << std::endl;
		int ret = system(cmd.c_str());
		if (ret == 127 || ret == -1)
			std::cout << "system error:" << ret << std::endl;
	}

	if (len == 1) {
		snprintf(tmp, 30, "tmp_%.3d.mp4", len);
		rename(tmp, _outputPath.c_str());
		return;
	}

	std::string concat = "/usr/bin/ffmpeg -y";
	for (int i = 1; i <= len; i++) {
		concat += " -i ";
		snprintf(tmp, 30, "tmp_%.3d.mp4", i);
		concat += tmp;
	}
	concat += " -filter_complex \"";
	for (int i = 0; i < len; i++) {
		snprintf(tmp, 30, "[%d:v] [%d:a] ", i%100, i%100);
		concat += tmp;
	}
	concat += "concat=n=";
	concat += std::to_string(len);
	concat += ":v=1:a=1 [vv] [aa]\" -map \"[vv]\" -map \"[aa]\" ";
	concat += _outputPath;

	std::cout << "ffmpegRender concat:" << concat << std::endl;
	int ret = system(concat.c_str());
	if (ret == 127 || ret == -1)
		std::cout << "system error:" << ret << std::endl;

	for (int i = 1; i <= len; i++) {
		snprintf(tmp, 30, "tmp_%.3d.mp4", i);
		remove(tmp);
	}
}

void FFmpegRender::render(timelineRetainer otioTimeline)
{
	if (_outputPath.empty())
		throw ovi::Exception(OVI_ERROR_INVALID_PARAMETER, "Invalid outputPath");

	_otioTimeline = otioTimeline;
	// TEMP: For debugging
	_otioTimeline->to_json_file(_outputPath + ".otio");

	_effect = std::make_unique<FFmpegEffect>(_otioTimeline);
	_effectOutputPath = _effect->outputPath();
	cut();

	if (!_effectOutputPath.empty())
		remove(_effectOutputPath.c_str());
}

MetaForm FFmpegRender::effectMetaForm(const std::string& effectName)
{
	return FFmpegEffectSpec::inputMetaForm(effectName);
}

std::string FFmpegRender::makeDescription()
{
	std::string desc = "This plugin supports simple cut and merge. \n";
	std::string attrStr = "\t\t attribute : ";

	desc += "\tSupported effects: \n";

	std::map<std::string, std::string> descMap = FFmpegEffectSpec::descriptions();
	std::map<std::string, std::vector<AttributeSpec>> attrsMap = FFmpegEffectSpec::attributes();

	for (const auto& [effectName, effectDesc] : descMap) {
		desc += "\t" + effectName + " : " + effectDesc + "\n";

		auto attrs = attrsMap.find(effectName);
		if (attrs == attrsMap.end())
			continue;

		for (const auto& attr : attrs->second) {
			desc += attrStr;
			desc += attr.name;
			if (!attr.spec.empty())
				desc += ", " + attr.spec;
			if (!attr.defaultValue.empty())
				desc += ", Default : " + attr.defaultValue;
			desc += "\n";
		}

		desc += "\n";
	}

	return desc;
}

extern "C" class IPlugin *createPlugin(void)
{
	return new FFmpegRender();
}

extern "C" void destroyPlugin(class IPlugin *render)
{
	delete render;
}

extern "C" const char *name()
{
	return "FFMPEGRender";
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
	return METAFORM_NONE;	//ToDo
}

extern "C" const char *description()
{
	static std::string desc;

	if (desc.empty())
		desc = FFmpegRender::makeDescription();

	return desc.c_str();
}

extern "C" void *attributeList()
{
	static std::vector<Attribute> attrs {
		{ "path", "string", "output file path" },
	};

	return &attrs;
}
