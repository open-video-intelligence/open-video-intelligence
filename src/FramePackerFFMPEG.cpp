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

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

#include "FramePackerFFMPEG.h"
#include "Log.h"
#include "Exception.h"

using namespace ovi;

static VideoFormat toVideoFormat(AVPixelFormat format)
{
	switch (format) {
	case AV_PIX_FMT_YUV420P:
		return VIDEO_FORMAT_YUV420P;

	case AV_PIX_FMT_YUV422P:
		return VIDEO_FORMAT_YUV422P;

	case AV_PIX_FMT_RGB24:
		return VIDEO_FORMAT_RGB24;

	case AV_PIX_FMT_BGR24:
		return VIDEO_FORMAT_BGR24;

	case AV_PIX_FMT_NV12:
		return VIDEO_FORMAT_NV12;

	case AV_PIX_FMT_NV21:
		return VIDEO_FORMAT_NV21;

	case AV_PIX_FMT_ARGB:
		return VIDEO_FORMAT_ARGB;

	case AV_PIX_FMT_RGBA:
		return VIDEO_FORMAT_RGBA;

	case AV_PIX_FMT_ABGR:
		return VIDEO_FORMAT_ABGR;

	case AV_PIX_FMT_BGRA:
		return VIDEO_FORMAT_BGRA;

	case AV_PIX_FMT_GRAY8:
		return VIDEO_FORMAT_GRAY8;

	default:
		LOG_ERROR("not supported AVPixelFormat: %d", format);
		return VIDEO_FORMAT_NONE;
	}
}

static AudioFormat toAudioFormat(AVSampleFormat format)
{
	switch (format) {
	case AV_SAMPLE_FMT_U8:
		return AUDIO_FORMAT_U8;

	case AV_SAMPLE_FMT_S16:
		return AUDIO_FORMAT_S16;

	case AV_SAMPLE_FMT_S32:
		return AUDIO_FORMAT_S32;

	case AV_SAMPLE_FMT_FLT:
		return AUDIO_FORMAT_FLT;

	case AV_SAMPLE_FMT_DBL:
		return AUDIO_FORMAT_DBL;

	case AV_SAMPLE_FMT_S64:
		return AUDIO_FORMAT_S64;

	case AV_SAMPLE_FMT_U8P:
		return AUDIO_FORMAT_U8P;

	case AV_SAMPLE_FMT_S16P:
		return AUDIO_FORMAT_S16P;

	case AV_SAMPLE_FMT_S32P:
		return AUDIO_FORMAT_S32P;

	case AV_SAMPLE_FMT_FLTP:
		return AUDIO_FORMAT_FLTP;

	case AV_SAMPLE_FMT_DBLP:
		return AUDIO_FORMAT_DBLP;

	case AV_SAMPLE_FMT_S64P:
		return AUDIO_FORMAT_S64P;

	default:
		LOG_ERROR("not supported AVSampleFormat: %d", format);
		return AUDIO_FORMAT_NONE;
	}
}

FramePackPtr FramePackerAvVideo::pack(void* srcFrame, size_t frameNum, double pts, double framerate, int64_t duration)
{
	auto frame = static_cast<AVFrame*>(srcFrame);
	auto format = static_cast<AVPixelFormat>(frame->format);

	int bufferSize = av_image_get_buffer_size(format, frame->width, frame->height, 1);
	if (bufferSize <= 0) {
		LOG_ERROR("failed to av_image_get_buffer_size(). format:%d width:%d height:%d",
				frame->format, frame->width, frame->height);
		return nullptr;
	}

	VideoFormat oviFormat = toVideoFormat(format);
	if (oviFormat == VIDEO_FORMAT_NONE) {
		//ToDo. Convert format
		LOG_ERROR("Not supported format. need to convert format");
		return nullptr;
	}

	void* buffer = av_malloc(bufferSize);
	if (!buffer) {
		LOG_ERROR("failed to av_malloc()");
		return nullptr;
	}

	av_image_copy_to_buffer(static_cast<uint8_t*>(buffer), bufferSize, frame->data, frame->linesize,
							format, frame->width, frame->height, 1);

	auto newFrame = std::make_unique<VideoFramePack>(frame->width, frame->height, oviFormat);
	newFrame->assign(buffer, bufferSize, frameNum, pts, framerate, duration);

#if 0 //Test
	newFrame->dump2Log(__FUNCTION__);
//	newFrame->dump2Log(std::string(__FUNCTION__) + ":" + std::to_string(__LINE__));
	newFrame->dump2File("./", true);
#endif

	av_freep(&buffer);
	return newFrame;
}

FramePackPtr FramePackerAvAudio::pack(void* srcFrame, size_t frameNum, double pts, double framerate, int64_t duration)
{
	auto frame = static_cast<AVFrame*>(srcFrame);
	auto format = static_cast<AVSampleFormat>(frame->format);
	int bytesPerSample = av_get_bytes_per_sample(format);

	AudioFormat oviFormat = toAudioFormat(format);
	if (oviFormat == AUDIO_FORMAT_NONE) {
		//ToDo. Convert format
		LOG_ERROR("Not supported format. need to convert format");
		return nullptr;
	}

	size_t bufferSize = bytesPerSample * frame->nb_samples * frame->channels;
	void* buffer = av_malloc(bufferSize);
	if (!buffer) {
		LOG_ERROR("failed to av_malloc()");
		return nullptr;
	}

	auto newFrame = std::make_unique<AudioFramePack>(frame->channels, frame->sample_rate, oviFormat, frame->nb_samples);
	newFrame->setChannelLayout(frame->channel_layout);

	if (av_sample_fmt_is_planar(format)) {
		size_t offset = 0;
		size_t planeSize = bytesPerSample * frame->nb_samples;
		for (int ch = 0; ch < frame->channels; ch++) {
			memcpy(static_cast<char*>(buffer) + offset, frame->data[ch], planeSize);
			offset += planeSize;
		}
		newFrame->assign(buffer, bufferSize, frameNum, pts, framerate, duration);
	} else {
		newFrame->assign(frame->data[0], bufferSize, frameNum, pts, framerate, duration);
	}

#if 0 //Test
	newFrame->dump2Log(__FUNCTION__);
//	newFrame->dump2Log(std::string(__FUNCTION__) + ":" + std::to_string(__LINE__));
	newFrame->dump2File("./", true);
#endif

	av_freep(&buffer);
	return newFrame;
}
