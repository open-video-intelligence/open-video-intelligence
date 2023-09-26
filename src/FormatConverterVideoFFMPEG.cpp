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

#include "FormatConverterVideoFFMPEG.h"
#include "Exception.h"
#include "Log.h"

using namespace ovi;

static AVPixelFormat toAVPixelFormat(VideoFormat format)
{
	switch (format) {
	case VIDEO_FORMAT_YUV420P:
		return AV_PIX_FMT_YUV420P;

	case VIDEO_FORMAT_YUV422P:
		return AV_PIX_FMT_YUV422P;

	case VIDEO_FORMAT_RGB24:
		return AV_PIX_FMT_RGB24;

	case VIDEO_FORMAT_BGR24:
		return AV_PIX_FMT_BGR24;

	case VIDEO_FORMAT_NV12:
		return AV_PIX_FMT_NV12;

	case VIDEO_FORMAT_NV21:
		return AV_PIX_FMT_NV21;

	case VIDEO_FORMAT_ARGB:
		return AV_PIX_FMT_ARGB;

	case VIDEO_FORMAT_RGBA:
		return AV_PIX_FMT_RGBA;

	case VIDEO_FORMAT_ABGR:
		return AV_PIX_FMT_ABGR;

	case VIDEO_FORMAT_BGRA:
		return AV_PIX_FMT_BGRA;

	case VIDEO_FORMAT_GRAY8:
		return AV_PIX_FMT_GRAY8;

	default:
		LOG_ERROR("invalid video format: %d", format);
		return AV_PIX_FMT_NONE;
	}
}


FormatConverterVideoFFMPEG::FormatConverterVideoFFMPEG()
{
	av_log_set_level(AV_LOG_QUIET);
}

FormatConverterVideoFFMPEG::~FormatConverterVideoFFMPEG()
{
}

FramePackPtr FormatConverterVideoFFMPEG::scale(const VideoFramePack* vFrame, VideoFormat dstFormat)
{
	void* data {};
	struct SwsContext* swsContext {};

	try {
		AVPixelArray src {};
		AVPixelArray dest {};
		auto [ width, height, format ] = vFrame->videoProperties();

		AVPixelFormat srcFormat = toAVPixelFormat(format);
		AVPixelFormat destFormat = toAVPixelFormat(dstFormat);

		LOG_INFO("Video Format: %d -> %d", srcFormat, destFormat);

		if (!vFrame->valid() || srcFormat == AV_PIX_FMT_NONE)
			throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid frame");

		char errStr[AV_ERROR_MAX_STRING_SIZE] {};

		int size = av_image_get_buffer_size(destFormat, width, height, 1);
		if (size <= 0)
			throw Exception(OVI_ERROR_INVALID_PARAMETER, av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, size));

		data = av_malloc(size);
		if (!data)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "out of memory");

		int ret = av_image_fill_arrays(src.slice, src.stride, (const uint8_t *)vFrame->data(),
					srcFormat, width, height, 1);
		if (ret < 0)
			throw Exception(OVI_ERROR_INVALID_OPERATION, av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, ret));

		ret = av_image_fill_arrays(dest.slice, dest.stride, (const uint8_t *)data,
					destFormat, width, height, 1);
		if (ret < 0)
			throw Exception(OVI_ERROR_INVALID_OPERATION, av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, ret));

		swsContext = sws_getContext(width, height, srcFormat,
						width, height, destFormat,
						SWS_BICUBIC, NULL, NULL, NULL);
		if (!swsContext)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "swscale error");

		ret = sws_scale(swsContext, (const uint8_t * const *)src.slice, src.stride,
					0, height, dest.slice, dest.stride);
		if (ret < 0)
			throw Exception(OVI_ERROR_INVALID_OPERATION, av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, ret));

		auto convertFrame = FramePackPtr(new VideoFramePack(width, height, dstFormat));
		convertFrame->assign(data, size, vFrame->frameNum(), vFrame->pts(), vFrame->framerate(), vFrame->duration());

		av_freep(&data);
		sws_freeContext(swsContext);

		return convertFrame;
	} catch (const Exception& e) {
		if (data)
			av_freep(&data);
		if (swsContext)
			sws_freeContext(swsContext);

		throw;
	}
}

FramePackPtr FormatConverterVideoFFMPEG::convert(const FramePack* frame, int format)
{
	const VideoFramePack* vFrame = dynamic_cast<const VideoFramePack*>(frame);
	assert(vFrame);

	VideoFormat srcFormat {};
	std::tie(std::ignore, std::ignore, srcFormat) = vFrame->videoProperties();

	auto dstFormat = static_cast<VideoFormat>(format);
	if (srcFormat == dstFormat)
		return FramePackPtr(new VideoFramePack(*vFrame));

	return scale(vFrame, dstFormat);
}

FramePackPtr FormatConverterVideoFFMPEG::convert(const FramePack* frame, const std::vector<int>& formats)
{
	if (!frame)
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid frame");

	if (formats.empty())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid formats");

	for (const auto& format : formats) {
		if (toAVPixelFormat(static_cast<VideoFormat>(format)) == AV_PIX_FMT_NONE)
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
