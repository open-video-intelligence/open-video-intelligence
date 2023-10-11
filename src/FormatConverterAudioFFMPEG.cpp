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

#include <iostream>
#include <cassert>

#include "FormatConverterAudioFFMPEG.h"
#include "Exception.h"
#include "Log.h"

using namespace ovi;

static AVSampleFormat toAVSampleFormat(AudioFormat format)
{
	switch (format) {
	case AUDIO_FORMAT_U8:
		return AV_SAMPLE_FMT_U8;

	case AUDIO_FORMAT_S16:
		return AV_SAMPLE_FMT_S16;

	case AUDIO_FORMAT_S32:
		return AV_SAMPLE_FMT_S32;

	case AUDIO_FORMAT_FLT:
		return AV_SAMPLE_FMT_FLT;

	case AUDIO_FORMAT_DBL:
		return AV_SAMPLE_FMT_DBL;

	case AUDIO_FORMAT_S64:
		return AV_SAMPLE_FMT_S64;

	case AUDIO_FORMAT_U8P:
		return AV_SAMPLE_FMT_U8P;

	case AUDIO_FORMAT_S16P:
		return AV_SAMPLE_FMT_S16P;

	case AUDIO_FORMAT_S32P:
		return AV_SAMPLE_FMT_S32P;

	case AUDIO_FORMAT_FLTP:
		return AV_SAMPLE_FMT_FLTP;

	case AUDIO_FORMAT_DBLP:
		return AV_SAMPLE_FMT_DBLP;

	case AUDIO_FORMAT_S64P:
		return AV_SAMPLE_FMT_S64P;

	default:
		LOG_ERROR("invalid audio format: %d", format);
		return AV_SAMPLE_FMT_NONE;
	}
}

FormatConverterAudioFFMPEG::FormatConverterAudioFFMPEG()
{
}

FormatConverterAudioFFMPEG::~FormatConverterAudioFFMPEG()
{

}

FramePackPtr FormatConverterAudioFFMPEG::resample(const AudioFramePack* aFrame, AudioFormat dstFormat)
{
	uint8_t** srcData {};
	uint8_t** destData {};
	struct SwrContext* swrContext {};

	try {
		char errStr[AV_ERROR_MAX_STRING_SIZE] {};

		auto [ channels, samplerate, format, samples ] = aFrame->audioProperties();
		auto channelLayout = aFrame->channelLayout();

		AVSampleFormat avSrcFormat = toAVSampleFormat(format);
		AVSampleFormat avDestFormat = toAVSampleFormat(dstFormat);

		LOG_INFO("Audio Format: %d -> %d", avSrcFormat, avDestFormat);

		if (!aFrame->valid() || avSrcFormat == AV_SAMPLE_FMT_NONE)
			throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid frame");

		if (channelLayout == 0)
			throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid channel-layout");

		int srcPlanes = (av_sample_fmt_is_planar(avSrcFormat) ? channels : 1);
		srcData = (uint8_t**)calloc(srcPlanes, sizeof(uint8_t*));
		if (!srcData)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "out of memory");

		int ret = av_samples_fill_arrays(srcData, nullptr, static_cast<const uint8_t*>(aFrame->data()), channels, samples, avSrcFormat, 0);
		if (ret < 0)
			throw Exception(OVI_ERROR_INVALID_OPERATION, av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, ret));

		int size = av_samples_alloc_array_and_samples(&destData, nullptr, channels, samples, avDestFormat, 0);
		if (size < 0)
			throw Exception(OVI_ERROR_INVALID_OPERATION, av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, size));

		swrContext = swr_alloc_set_opts(nullptr,           // allocating a new context
						channelLayout, avDestFormat, samplerate,  // dest ch_layout, format, samplerate
						channelLayout, avSrcFormat, samplerate,   // src ch_layout, format, samplerate
						0, nullptr);                        // for logger

		if (!swrContext)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "swresample error");

		ret = swr_init(swrContext);
		if (ret < 0)
			throw Exception(OVI_ERROR_INVALID_OPERATION, av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, ret));

		ret = swr_convert(swrContext, destData, samples, (const uint8_t **)srcData, samples);
		if (ret < 0)
			throw Exception(OVI_ERROR_INVALID_OPERATION, av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, ret));

		auto convertFrame = FramePackPtr(new AudioFramePack(channels, samplerate, dstFormat, samples));
		convertFrame->assign((void*)destData[0], size, aFrame->frameNum(), aFrame->pts(), aFrame->framerate(), aFrame->duration());
		auto aFrame = dynamic_cast<AudioFramePack*>(convertFrame.get());
		assert(aFrame);
		aFrame->setChannelLayout(channelLayout);

		free(srcData);
		srcData = nullptr;

		if (destData[0])
			av_freep(&destData[0]);
		av_freep(&destData);

		swr_free(&swrContext);

		return convertFrame;
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());

		if (srcData) {
			free(srcData);
			srcData = nullptr;
		}
		if (destData) {
			if (destData[0])
				av_freep(&destData[0]);
			av_freep(&destData);
		}
		if (swrContext)
			swr_free(&swrContext);

		throw;
	}
}

FramePackPtr FormatConverterAudioFFMPEG::convert(const FramePack* frame, int format)
{
	const AudioFramePack* aFrame = dynamic_cast<const AudioFramePack*>(frame);
	assert(aFrame);

	AudioFormat srcFormat {};
	std::tie(std::ignore, std::ignore, srcFormat, std::ignore) = aFrame->audioProperties();

	auto dstFormat = static_cast<AudioFormat>(format);
	if (srcFormat == dstFormat)
		return FramePackPtr(new AudioFramePack(*aFrame));

	return resample(aFrame, dstFormat);
}

FramePackPtr FormatConverterAudioFFMPEG::convert(const FramePack* frame, const std::vector<int>& formats)
{
	if (!frame)
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid frame");

	if (formats.empty())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid formats");

	for (const auto& format : formats) {
		if (toAVSampleFormat(static_cast<AudioFormat>(format)) == AV_SAMPLE_FMT_NONE)
			continue;

		try {
			return convert(frame, format);
		} catch (const Exception& e) {
			if (e.error() != OVI_ERROR_INVALID_OPERATION)
				throw;
		}
	}
	throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid formats");
}
