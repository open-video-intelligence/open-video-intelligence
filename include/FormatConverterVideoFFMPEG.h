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

#ifndef __OPEN_VIDEO_INTELLIGENCE_FORMAT_CONVERTER_VIDEO_FFMPEG_H__
#define __OPEN_VIDEO_INTELLIGENCE_FORMAT_CONVERTER_VIDEO_FFMPEG_H__

extern "C" {
#include <libavutil/log.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
//#include <libavutil/samplefmt.h>
}

#include "IFormatConverter.h"
#include "FramePack.h"

namespace ovi {

class FormatConverterVideoFFMPEG : public IFormatConverter
{
public:
	FormatConverterVideoFFMPEG();
	~FormatConverterVideoFFMPEG() override;

	FramePackPtr convert(const FramePack* frame, const std::vector<int>& formats) override;

private:
	FramePackPtr convert(const FramePack* frame, int format);
	FramePackPtr scale(const VideoFramePack* vFrame, VideoFormat dstFormat);

	static constexpr size_t PIXEL_DATA_CHANNEL_NUM = 4;

	struct AVPixelArray {
		uint8_t* slice[PIXEL_DATA_CHANNEL_NUM] {};
		int stride[PIXEL_DATA_CHANNEL_NUM] {};
	};

};

} // namespace

#endif // __OPEN_VIDEO_INTELLIGENCE_FORMAT_CONVERTER_VIDEO_FFMPEG_H__
