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

#ifndef __OPEN_VIDEO_INTELLIGENCE_FFMPEG_EFFECT_H__
#define __OPEN_VIDEO_INTELLIGENCE_FFMPEG_EFFECT_H__

#include <memory>
#include <string>
#include "IPluginRender.h"
#include "AttributeValidator.h"

struct Timeline {
	double startSec {};
	double endSec {};
};

struct CoordinateInfo {
	double x {};
	double y {};
	double w {};
	double h {};
	Timeline time;
};

struct StringInfo {
	std::string value {};
	Timeline time;
};

struct DoubleInfo {
	double value {};
	Timeline time;
};

struct CmdGenerator
{
	static std::string makeVideoBlurEffectCmd(const std::string& inputPath, const std::string& outputPath, const std::string& blurName, int intensity, std::vector<CoordinateInfo> coordInfo);
	static std::string makeVideoBoxEffectCmd(const std::string& inputPath, const std::string& outputPath, const std::string& color, int thickness, std::vector<CoordinateInfo> coordInfo);
	static std::string makeVideoEqEffectCmd(const std::string& inputPath, const std::string& outputPath, const std::string& effectName, double effectValue, std::vector<Timeline> timeInfo);
	static std::string makeVideoTextEffectCmd(const std::string& inputPath, const std::string& outputPath, const std::string& fontColor, const std::string& fontSize,
		int x, int y, std::vector<StringInfo> strInfo);
	static std::string makeVideoStickerEffectCmd(const std::string& inputPath, const std::string& outputPath, const std::string& imgPath, std::vector<CoordinateInfo> _coordInfo);
	static std::string makeAudioVolumeEffectCmd(const std::string& inputPath, const std::string& outputPath, double volume, std::vector<Timeline> _timeInfo);
};

struct FFmpegEffectSpec
{
	static std::map<std::string, std::string> descriptions();
	static std::map<std::string, std::vector<AttributeSpec>> attributes();
	static std::string effectAttrDefaultValue(const std::string& effectName, const std::string& attrName);
	static MetaForm inputMetaForm(const std::string& effectName);
};

class FFmpegEffect
{
public:
	explicit FFmpegEffect(timelineRetainer otioTimeline);
	~FFmpegEffect() = default;

	std::string outputPath() const { return _outputPath; }

private:
	void applyEffect(const std::string& inputPath, const std::string& outputPath, const std::string& effectName, otio::AnyDictionary& metadata, double rate);
	void setOutputPath(const std::string& effectName);
	std::vector<CoordinateInfo> getCoordInfo(otio::AnyDictionary& metadata, double rate);
	std::vector<StringInfo> getConvertedStringInfo(otio::AnyDictionary& metadata, double rate);
	std::vector<DoubleInfo> getDoubleInfo(otio::AnyDictionary& metadata, double rate);
	std::vector<Timeline> getTimeline(otio::AnyDictionary& metadata, double rate);

	std::string popItem(otio::AnyDictionary& metadata, const std::string& key, const std::string& defaultValue);

	std::string _outputPath;
};
#endif // __OPEN_VIDEO_INTELLIGENCE_FFMPEG_EFFECT_H__
