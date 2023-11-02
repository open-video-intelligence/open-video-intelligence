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

#ifndef __OPEN_VIDEO_INTELLIGENCE_FRAME_EXTRACTOR_FFMPEG_H__
#define __OPEN_VIDEO_INTELLIGENCE_FRAME_EXTRACTOR_FFMPEG_H__


#include <string>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#include "IFrameExtractor.h"
#include "FramePackerFactory.h"
#include "MediaInfoFFMPEG.h"

namespace ovi {

class AvDecoder
{
public:
	AvDecoder(AVMediaType mediaType, int streamId, std::string mediaPath,
				IFramePackerPtr packer);
	virtual ~AvDecoder();

	FramePackPtr frame(double framerate, int64_t duration);
	AVFrame* decode();
	size_t frameNum() const;

private:
	AVPacket* readFrame();
	void ready();

	AVFormatContext*_formatCtx {};
	AVCodecContext*_codecCtx {};
	AVMediaType _mediaType {};
	int _streamId {};
	size_t _frameNum {};
	AVRational _time_base {};
	bool _eof {};

	std::unique_ptr<IFramePacker> _packer;
};

struct AvDecoderFactory
{
	static std::unique_ptr<AvDecoder> createAudioDecoder(int streamId, const std::string& mediaPath);
	static std::unique_ptr<AvDecoder> createVideoDecoder(int streamId, const std::string& mediaPath);
};

class FrameExtractorFFMPEG : public IFrameExtractor
{
public:
	explicit FrameExtractorFFMPEG(const std::string& media_path);
	~FrameExtractorFFMPEG() override = default;

	FramePackPtr nextVideo() const override;
	FramePackPtr nextAudio() const override;

	MediaInfoPtr mediaInfo() const override;

private:
	void setup(const std::string& mediaPath);

	std::shared_ptr<MediaInfoFFMPEG> _mediaInfo;
	std::unique_ptr<AvDecoder> _videoDecoder;
	std::unique_ptr<AvDecoder> _audioDecoder;
};

}

#endif // __OPEN_VIDEO_INTELLIGENCE_FRAME_EXTRACTOR_FFMPEG_H__
