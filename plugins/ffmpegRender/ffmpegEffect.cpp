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

#include <filesystem>
#include <unistd.h>

#include "ffmpegEffect.h"

std::string CmdGenerator::makeVideoBlurEffectCmd(const std::string& inputPath, const std::string& outputPath,
												const std::string& blurName, int intensity,
												std::vector<CoordinateInfo> coordInfo)
{
	std::string cmd = "/usr/bin/ffmpeg -y -i ";
	cmd += inputPath;
	cmd += " -filter_complex \"";

	size_t _coordSize = coordInfo.size();
	char tmp[256];

	for (size_t i = 0; i < _coordSize; i++) {
		snprintf(tmp, 256, "[0:v]crop=%f:%f:%f:%f,%s=%d[crop%zu]; ",
			coordInfo[i].w, coordInfo[i].h, coordInfo[i].x, coordInfo[i].y, blurName.c_str(), intensity, i + 1);

		cmd += tmp;
	}

	for (size_t i = 0; i < _coordSize; i++) {
		if (_coordSize == 1)	 {
			snprintf(tmp, 256, "[0:v][crop%zu]overlay=%f:%f:enable='between(t,%f,%f)' ",
				i + 1, coordInfo[i].x, coordInfo[i].y, coordInfo[i].time.startSec, coordInfo[i].time.endSec);

			cmd += tmp;
			break;
		}

		if (i == 0) {
			snprintf(tmp, 256, "[0:v][crop%zu]overlay=%f:%f:enable='between(t,%f,%f)' [ovr%zu]; ",
				i + 1, coordInfo[i].x, coordInfo[i].y, coordInfo[i].time.startSec, coordInfo[i].time.endSec, i + 1);
		} else if (i == _coordSize - 1) {
			snprintf(tmp, 256, "[ovr%zu][crop%zu]overlay=%f:%f:enable='between(t,%f,%f)' ",
				i, i + 1, coordInfo[i].x, coordInfo[i].y, coordInfo[i].time.startSec, coordInfo[i].time.endSec);
		} else {
			snprintf(tmp, 256, "[ovr%zu][crop%zu]overlay=%f:%f:enable='between(t,%f,%f)' [ovr%zu]; ",
				i, i + 1, coordInfo[i].x, coordInfo[i].y, coordInfo[i].time.startSec, coordInfo[i].time.endSec, i + 1);
		}

		cmd += tmp;
	}

	cmd += "\" -movflags +faststart ";
	cmd += outputPath;

	std::cout << "blur effect cmd:" << cmd << std::endl;

	return cmd;
}

std::string CmdGenerator::makeVideoBoxEffectCmd(const std::string& inputPath, const std::string& outputPath,
												const std::string& color, int thickness,
												std::vector<CoordinateInfo> coordInfo)
{
	std::string cmd = "/usr/bin/ffmpeg -y -i ";
	cmd += inputPath;
	cmd += " -vf \"";

	for (size_t i = 0; i < coordInfo.size(); i++) {
		char tmp[256];
		snprintf(tmp, 256, "drawbox=x=%f:y=%f:w=%f:h=%f:enable='between(t,%f,%f)':c=%s:t=%d",
			coordInfo[i].x, coordInfo[i].y, coordInfo[i].w, coordInfo[i].h,
			coordInfo[i].time.startSec, coordInfo[i].time.endSec, color.c_str(), thickness);

		cmd += tmp;
		if (i != coordInfo.size() - 1)
			cmd += ",";
	}

	cmd += "\" ";
	cmd += outputPath;

	std::cout << "box effect cmd:" << cmd << std::endl;

	return cmd;
}

std::string CmdGenerator::makeVideoEqEffectCmd(const std::string& inputPath, const std::string& outputPath,
											const std::string& effectName, double effectValue,
											std::vector<Timeline> timeInfo)
{
	std::string cmd = "/usr/bin/ffmpeg -y -i ";
	cmd += inputPath;
	cmd += " -vf \"eq=";

	char tmp[256];
	snprintf(tmp, 256, "%s=%f:", effectName.c_str(), effectValue);
	cmd += tmp;
	cmd += "enable='";

	for (size_t i = 0; i < timeInfo.size(); i++) {
		if (i == 0)
			snprintf(tmp, 256, "between(t,%f,%f)", timeInfo[i].startSec, timeInfo[i].endSec);
		else
			snprintf(tmp, 256, "+between(t,%f,%f)", timeInfo[i].startSec, timeInfo[i].endSec);
		cmd += tmp;
	}

	cmd += "'\" ";
	cmd += outputPath;

	std::cout << effectName << " effect cmd:" << cmd << std::endl;

	return cmd;
}

std::string CmdGenerator::makeVideoTextEffectCmd(const std::string& inputPath, const std::string& outputPath,
												const std::string& fontColor, const std::string& fontSize,
												int x, int y,
												std::vector<StringInfo> strInfo)

{
	std::string cmd = "/usr/bin/ffmpeg -y -i ";
	cmd += inputPath;
	cmd += " -vf \"";

	for (size_t i = 0; i < strInfo.size(); i++) {
		char tmp[256];

		snprintf(tmp, 256, "drawtext=text=%s:enable='between(t,%f,%f)':fontcolor=%s:fontsize=%s:x=%d:y=%d",
			strInfo[i].value.c_str(), strInfo[i].time.startSec, strInfo[i].time.endSec, fontColor.c_str(), fontSize.c_str(), x, y);

		cmd += tmp;
		if (i != strInfo.size() - 1)
			cmd += ",";
	}

	cmd += "\" ";
	cmd += outputPath;

	std::cout << "drawtext effect cmd:" << cmd << std::endl;

	return cmd;
}

std::string CmdGenerator::makeVideoStickerEffectCmd(const std::string& inputPath, const std::string& outputPath,
													const std::string& imgPath,
													std::vector<CoordinateInfo> coordInfo)
{
	std::string cmd = "/usr/bin/ffmpeg -y -i ";
	cmd += inputPath;
	cmd += " -i ";
	cmd += imgPath;
	cmd += " -filter_complex \"";

	size_t coordSize = coordInfo.size();
	char tmp[256];

	for (size_t i = 0; i < coordSize; i++) {
		double scaleSize = ( coordInfo[i].w > coordInfo[i].h) ? coordInfo[i].w : coordInfo[i].h;	//ToDo. The ratio should be considered

		if (i == 0) {
			snprintf(tmp, 256, "[1:v]scale=%f:%f[s%zu], [0:v][s%zu]overlay=%f:%f:enable='between(t,%f,%f)' [ovr%zu] ",
				scaleSize, scaleSize, i + 1, i + 1, coordInfo[i].x, coordInfo[i].y, coordInfo[i].time.startSec, coordInfo[i].time.endSec, i + 1);
		} else if (i == coordSize - 1) {
			snprintf(tmp, 256, "[1:v]scale=%f:%f[s%zu], [ovr%zu][s%zu]overlay=%f:%f:enable='between(t,%f,%f)' ",
				scaleSize, scaleSize, i + 1, i, i + 1, coordInfo[i].x, coordInfo[i].y, coordInfo[i].time.startSec, coordInfo[i].time.endSec);
		} else {
			snprintf(tmp, 256, "[1:v]scale=%f:%f[s%zu], [ovr%zu][s%zu]overlay=%f:%f:enable='between(t,%f,%f)' [ovr%zu] ",
				scaleSize, scaleSize, i + 1, i, i + 1, coordInfo[i].x, coordInfo[i].y, coordInfo[i].time.startSec, coordInfo[i].time.endSec, i + 1);
		}

		cmd += tmp;

		if (i != coordInfo.size() - 1)
			cmd += ";";
	}

	cmd += "\" ";
	cmd += outputPath;

	std::cout << "sticker effect cmd:" << cmd << std::endl;

	return cmd;
}

std::string CmdGenerator::makeAudioVolumeEffectCmd(const std::string& inputPath, const std::string& outputPath,
													double volume,
													std::vector<Timeline> timeInfo)
{
	std::string cmd = "/usr/bin/ffmpeg -y -i ";
	cmd += inputPath;
	cmd += " -af \"volume=";

	char tmp[256];
	snprintf(tmp, 256, "volume=%f:", volume);
	cmd += tmp;
	cmd += "enable='";

	for (size_t i = 0; i < timeInfo.size(); i++) {
		if (i == 0)
			snprintf(tmp, 256, "between(t,%f,%f)", timeInfo[i].startSec, timeInfo[i].endSec);
		else
			snprintf(tmp, 256, "+between(t,%f,%f)", timeInfo[i].startSec, timeInfo[i].endSec);
		cmd += tmp;
	}

	cmd += "'\" ";
	cmd += outputPath;

	std::cout << "volume effect cmd:" << cmd << std::endl;

	return cmd;
}

FFmpegEffect::FFmpegEffect(timelineRetainer otioTimeline)
{
	auto clips = otioTimeline->find_clips();
	if (clips.empty()) {
		std::cout << "No clips" << std::endl;
		return;
	}

	std::string inputPath;

	for (const auto& clip : clips) {
		auto& effects = clip.value->effects();

		if (effects.empty()) {
			std::cout << "No effect" << std::endl;
			continue;
		}

		auto ex = dynamic_cast<otio::ExternalReference*>(clip->media_reference());
		assert(ex);
		auto targetUrl = ex->target_url();

		if (inputPath.empty())
			inputPath = targetUrl;

		for (const auto& effect : effects) {
			if (!_outputPath.empty())
				inputPath = _outputPath;

			setOutputPath(effect->effect_name());
			applyEffect(inputPath, _outputPath,
						effect->effect_name(), effect->metadata(),
						clip->trimmed_range().duration().rate());

			if (inputPath != targetUrl)
				remove(inputPath.c_str());
		}
	}
}

std::string FFmpegEffect::popItem(otio::AnyDictionary& metadata, const std::string& key, const std::string& defaultValue)
{
	std::string value = defaultValue;

	metadata.get_if_set(key, &value);
	std::cout << key << ":" << value << std::endl;
	metadata.erase(key);

	return value;
}

void FFmpegEffect::applyEffect(const std::string& inputPath, const std::string& outputPath,
								const std::string& effectName,
								otio::AnyDictionary& metadata, double rate)
{
	std::string cmd;

	if (effectName == "boxblur" ||
		effectName == "avgblur") {
		int intensity = std::stoi(popItem(metadata, "intensity", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "intensity"))); // 0 to 19
		cmd = CmdGenerator::makeVideoBlurEffectCmd(inputPath, outputPath,
												effectName, intensity,
												getCoordInfo(metadata, rate));
	} else if (effectName == "drawbox") {
		std::string color = popItem(metadata, "color", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "color"));
		int thickness = std::stoi(popItem(metadata, "thickness", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "thickness")));
		cmd = CmdGenerator::makeVideoBoxEffectCmd(inputPath, outputPath,
												color, thickness,
												getCoordInfo(metadata, rate));
	} else if (effectName == "brightness") {
		double value = std::stof(popItem(metadata, "value", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "value"))); // -1.0 to 1.0
		cmd = CmdGenerator::makeVideoEqEffectCmd(inputPath, outputPath,
												effectName, value,
												getTimeline(metadata, rate));
	} else if (effectName == "contrast") {
		double value = std::stof(popItem(metadata, "value", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "value"))); // -1000.0 to 1000.0
		cmd = CmdGenerator::makeVideoEqEffectCmd(inputPath, outputPath,
												effectName, value,
												getTimeline(metadata, rate));
	} else if (effectName == "saturation") {
		double value = std::stof(popItem(metadata, "value", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "value"))); // 0.0 to 3.0
		cmd = CmdGenerator::makeVideoEqEffectCmd(inputPath, outputPath,
												effectName, value,
												getTimeline(metadata, rate));
	} else if (effectName == "drawtext") {
		std::string fontcolor = popItem(metadata, "fontcolor", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "fontcolor"));
		std::string fontsize = popItem(metadata, "fontsize", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "fontsize"));
		int x = std::stoi(popItem(metadata, "x", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "x")));
		int y = std::stoi(popItem(metadata, "y", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "y")));
		cmd = CmdGenerator::makeVideoTextEffectCmd(inputPath, outputPath,
												fontcolor, fontsize, x, y,
												getConvertedStringInfo(metadata, rate));
	} else if (effectName == "sticker") {
		std::string imgPath = popItem(metadata, "path", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "path"));
		cmd = CmdGenerator::makeVideoStickerEffectCmd(inputPath, outputPath,
													imgPath,
													getCoordInfo(metadata, rate));
	} else if (effectName == "volume") {
		double volume = std::stof(popItem(metadata, "volume", FFmpegEffectSpec::effectAttrDefaultValue(effectName, "volume")));
		cmd = CmdGenerator::makeAudioVolumeEffectCmd(inputPath, outputPath, volume, getTimeline(metadata, rate));
	} else {
		std::cout << "Not supported effect:" << effectName << std::endl;
		return;
	}

	int ret = system(cmd.c_str());
	if (ret == 127 || ret == -1)
		std::cout << "system error:" << ret << std::endl;
}

std::vector<CoordinateInfo> FFmpegEffect::getCoordInfo(otio::AnyDictionary& metadata, double rate)
{
	std::vector<CoordinateInfo> coordInfo;

	for (const auto& [ key, value ] : metadata) {
		Timeline timeLine {
			otio::RationalTime(std::stof(key), rate).to_seconds(),
			otio::RationalTime(std::stof(key) + 1, rate).to_seconds()
		};

		for (const auto& anyRect : std::any_cast<otio::AnyVector>(value)) {
			auto rect = std::any_cast<otio::AnyDictionary>(anyRect);

			coordInfo.push_back({
				std::any_cast<double>(rect["x"]),
				std::any_cast<double>(rect["y"]),
				std::any_cast<double>(rect["width"]),
				std::any_cast<double>(rect["height"]),
				timeLine
			});
		}
	}

	return coordInfo;
}

static std::string __strConverter(std::any& value)
{
	if (value.type() == typeid(int)) {
		return std::to_string(std::any_cast<int>(value));

	} else if (value.type() == typeid(long)) {
		return std::to_string(std::any_cast<long>(value));

	} else if (value.type() == typeid(float)) {
		return std::to_string(std::any_cast<float>(value));

	} else if (value.type() == typeid(double)) {
		return std::to_string(std::any_cast<double>(value));

	} else if (value.type() == typeid(std::string)) {
		return std::any_cast<std::string>(value);

	} else {
		std::cout << "not supported any type. name : " << value.type().name() << std::endl;
		return "";
	}
}

std::vector<StringInfo> FFmpegEffect::getConvertedStringInfo(otio::AnyDictionary& metadata, double rate)
{
	std::vector<StringInfo> stringInfo;

	for (const auto& [ key, value ] : metadata) {
		Timeline timeLine {
			otio::RationalTime(std::stof(key), rate).to_seconds(),
			otio::RationalTime(std::stof(key) + 1, rate).to_seconds()
		};

		for (const auto& anyValue : std::any_cast<otio::AnyVector>(value)) {
			auto dbl = std::any_cast<otio::AnyDictionary>(anyValue);
			stringInfo.push_back({
				__strConverter(dbl["value"]),
				timeLine
			});
		}
	}

	return stringInfo;
}

std::vector<DoubleInfo> FFmpegEffect::getDoubleInfo(otio::AnyDictionary& metadata, double rate)
{
	std::vector<DoubleInfo> doubleInfo;

	for (const auto& [ key, value ] : metadata) {
		Timeline timeLine {
			otio::RationalTime(std::stof(key), rate).to_seconds(),
			otio::RationalTime(std::stof(key) + 1, rate).to_seconds()
		};

		for (const auto& anyValue : std::any_cast<otio::AnyVector>(value)) {
			auto dbl = std::any_cast<otio::AnyDictionary>(anyValue);

			doubleInfo.push_back({
				std::any_cast<double>(dbl["value"]),
				timeLine
			});
		}
	}

	return doubleInfo;
}

std::vector<Timeline> FFmpegEffect::getTimeline(otio::AnyDictionary& metadata, double rate)
{
	std::vector<Timeline> timeInfo;

	for (const auto& [ key, value ] : metadata) {
		Timeline timeLine {
			otio::RationalTime(std::stof(key), rate).to_seconds(),
			otio::RationalTime(std::stof(key) + 1, rate).to_seconds()
		};

		timeInfo.push_back(timeLine);
	}

	return timeInfo;
}

void FFmpegEffect::setOutputPath(const std::string& effectName)
{
	static int fileNum = 0;

	_outputPath = {};

	_outputPath += "./effect_";
	_outputPath += effectName;
	_outputPath += "_";
	_outputPath += std::to_string(fileNum++);
	_outputPath += ".mp4";	//ToDo

	std::cout << "effect outputPath:" << _outputPath << std::endl;
}

std::map<std::string, std::string> FFmpegEffectSpec::descriptions()
{
	std::map<std::string, std::string> effectDescription {
		{ "boxblur", "Apply a boxblur algorithm to the input video. https://ffmpeg.org/ffmpeg-filters.html#toc-boxblur" },
		{ "avgblur", "Apply average blur filter. https://ffmpeg.org/ffmpeg-filters.html#avgblur" },
		{ "drawbox", "Draw a colored box on the input image. https://ffmpeg.org/ffmpeg-filters.html#toc-drawbox" },
		{ "drawtext", "Draw a text string. https://ffmpeg.org/ffmpeg-filters.html#drawtext-1" },
		{ "sticker", "Overlay image. https://ffmpeg.org/ffmpeg-filters.html#overlay-1" },
		{ "brightness", "Set the brightness expression. https://ffmpeg.org/ffmpeg-filters.html#eq" },
		{ "contrast", "Set the contrast expression. https://ffmpeg.org/ffmpeg-filters.html#eq" },
		{ "saturation", "Set the saturation expression. https://ffmpeg.org/ffmpeg-filters.html#eq" },
		{ "volume", "Adjust the input audio volume. https://ffmpeg.org/ffmpeg-filters.html#volume" }
	};

	return effectDescription;
}

std::map<std::string, std::vector<AttributeSpec>> FFmpegEffectSpec::attributes()
{
	std::map<std::string, std::vector<AttributeSpec>> attrsInfo;
	std::string rangeStr = "Supported range : ";

	attrsInfo["boxblur"] = {
		{ .name = "intensity",
		  .spec = rangeStr + "0 ~ 19",
		  .defaultValue = "10",
		  .validateFunc = [](std::string& s) -> bool {
				return (0.0 <= std::stof(s) && std::stof(s) <= 19.0);
			}
		}
	};

	attrsInfo["avgblur"] = {
		{ .name = "intensity",
		  .spec = rangeStr + "0 ~ 19",
		  .defaultValue = "10",
		  .validateFunc = [](std::string& s) -> bool {
				return (0.0 <= std::stof(s) && std::stof(s) <= 19.0);
			}
		}
	};

	attrsInfo["drawbox"] = {
		{ .name = "color",
		  .defaultValue = "green"
		},
		{ .name = "thickness",
		  .defaultValue = "3"
		}
	};

	attrsInfo["contrast"] = {
		{ .name = "value",
		  .spec = rangeStr + "-1000 ~ 1000",
		  .defaultValue = "1.0",
		  .validateFunc = [](std::string& s) -> bool {
				return (-1000.0 <= std::stof(s) && std::stof(s) <= 1000.0);
			}
		}
	};

	attrsInfo["brightness"] = {
		{ .name = "value",
		  .spec = rangeStr + "-1.0 ~ 1.0",
		  .defaultValue = "0.0",
		  .validateFunc = [](std::string& s) -> bool {
				return (-1.0 <= std::stof(s) && std::stof(s) <= 1.0);
			}
		}
	};

	attrsInfo["saturation"] = {
		{ .name = "value",
		  .spec = rangeStr + "0.0 ~ 3.0",
		  .defaultValue = "1.0",
		  .validateFunc = [](std::string& s) -> bool {
				return (0.0 <= std::stof(s) && std::stof(s) <= 3.0);
			}
		}
	};

	attrsInfo["drawtext"] = {
		{ .name = "fontcolor",
		  .defaultValue = "black"
		},
		{ .name = "fontsize",
		  .defaultValue = "16"
		},
		{ .name = "x",
		  .defaultValue = "0"
		},
		{ .name = "y",
		  .defaultValue = "0"
		}
	};

	attrsInfo["sticker"] = {
		{ .name = "path",
		  .mandatory = true,
		  .validateFunc = [](std::string& s) -> bool {
				// FIXME: this logic can be moved to util class
				std::error_code err;
				auto pathObj = std::filesystem::canonical(s, err);
				if (err.value() != 0)
					return false;

				std::string path = pathObj.string();
				if (access(path.c_str(), R_OK) < 0) {
					if (errno == EACCES || errno == EPERM) {
						std::cout << "Fail to open path: Permission Denied. path:" << path << std::endl;
						return false;
					}
					std::cout << "Fail to open path: Invalid Path. path:" << path << std::endl;
					return false;
				}
				return true;

			},
		}
	};

	attrsInfo["volume"] = {
		{ .name = "volume",
		  .defaultValue = "1.0"
		}
	};

	return attrsInfo;
}

std::string FFmpegEffectSpec::effectAttrDefaultValue(const std::string& effectName, const std::string& attrName)
{
	static std::map<std::string, std::vector<AttributeSpec>> attrsInfo = attributes();

	auto attrs = attrsInfo.find(effectName);
	if (attrs == attrsInfo.end()) {
		std::cout << "Not supported effect:" << effectName << std::endl;
		return {};
	}

	auto found = std::find_if(attrs->second.begin(), attrs->second.end(), [&attrName](const AttributeSpec& attr) {
		return attr.name == attrName;});

	if (found == attrs->second.end()) {
		std::cout << "Not supported effect attribute:" << attrName << std::endl;
		return {};
	}

	return found->defaultValue;
}

MetaForm FFmpegEffectSpec::inputMetaForm(const std::string& effectName)
{
	std::map<std::string, MetaForm> metaFormMap {
		{ "boxblur", METAFORM_RECT},
		{ "avgblur", METAFORM_RECT},
		{ "drawbox", METAFORM_RECT},
		{ "drawtext", METAFORM_ANY},	//ToDo. Change to allow proper multiple formats
		{ "sticker", METAFORM_RECT},
		{ "brightness", METAFORM_NONE},
		{ "contrast", METAFORM_NONE},
		{ "saturation", METAFORM_NONE},
		{ "volume", METAFORM_DOUBLE}
	};

	auto metaForm = metaFormMap.find(effectName);

	if (metaForm == metaFormMap.end())
		throw ovi::Exception(METAFORM_NONE, "Unsupported effect:" + effectName);

	return metaForm->second;
}
