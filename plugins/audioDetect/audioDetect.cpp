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

#include <cmath>
#include <cassert>
#include "IPluginProcess.h"

using namespace ovi;

class AudioDetect : public IPluginProcess
{
public:
	AudioDetect() = default;
	~AudioDetect() = default;

	int setAttrs(const std::map<std::string, std::string>& attrs) override;
	Outcome process(ovi::FramePack* frame) override;

private:
	double calculateRMS(const float* v, size_t l);
	double toDecibel(double rms);

	double _threshold { 60.0 };
	bool _inverse {};  // silence detect
};

double AudioDetect::toDecibel(double rms)
{
	constexpr float referencePressure = 0.00002f;

	return 20.0f * log10(rms / referencePressure);
}

double AudioDetect::calculateRMS(const float* v, size_t l)
{
	double sum = 0;

	for (size_t i = 0; i < l; i++)
		sum += pow(v[i], 2);

	return sqrt(sum / static_cast<double>(l));
}

int AudioDetect::setAttrs(const std::map<std::string, std::string>& attrs)
{
	if (attrs.find("threshold") != attrs.end())
		_threshold = std::stof(attrs.at("threshold"));

	if (attrs.find("inverse") != attrs.end())
		_inverse = (attrs.at("inverse") == "1");

	return OVI_ERROR_NONE;
}

Outcome AudioDetect::process(ovi::FramePack* frame)
{
	auto audioFrame = dynamic_cast<ovi::AudioFramePack *>(frame);
	assert(audioFrame);

	auto ptr = static_cast<float *>(const_cast<void*>(frame->data()));
	assert(ptr);

	auto [channels, samplerate, format, samples] = audioFrame->audioProperties();
	assert(format == AUDIO_FORMAT_FLTP);

	double db = toDecibel(calculateRMS(ptr, samples * channels));

	Outcome res { .detect = (db > _threshold), .list = {db} };
	if (_inverse)
		res.detect = !res.detect;

	if (res.detect)
		std::cout << "[" << frame->frameNum() << "] ++++ Detected Audio : " << db << " dB" << std::endl;

	return res;
}

extern "C" class IPlugin *createPlugin(void)
{
	return new AudioDetect();
}
extern "C" void destroyPlugin(class IPlugin *plugin)
{
	delete plugin;
}

extern "C" const char *name()
{
	return "AudioDetect";
}

extern "C" PluginType type()
{
	return PLUGIN_TYPE_AUDIO_DETECT;
}

// cppcheck-suppress unusedFunction
extern "C" void *supportFormat()
{
	static std::vector<int> formats {
		AUDIO_FORMAT_FLTP
	};

	return &formats;
}

extern "C" MetaForm supportMetaForm()
{
	return METAFORM_DOUBLE;
}

extern "C" const char *description()
{
	return "Detecting audio";
}

// cppcheck-suppress unusedFunction
extern "C" void *attributeList()
{
	static std::vector<Attribute> attrs {
		{"threshold", "double", "db threshold"},
		{"inverse", "0 or 1", "silence detect"},
	};

	return &attrs;
}
