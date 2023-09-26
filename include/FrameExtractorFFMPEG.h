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

class MediaInfo
{
protected:
	int _streamId { -1 };
	double _framerate {};
	int64_t _frames {};

public:
	virtual ~MediaInfo() = default;

	int streamId() const;
	bool hasStream() const;
	double framerate() const;
	int64_t frameCnt() const;
};

class VideoInfo : public MediaInfo
{
public:
	VideoInfo(AVFormatContext* formatCtx, const std::string& mediaPath);
	~VideoInfo() override = default;
};

class AudioInfo : public MediaInfo
{
public:
	AudioInfo(AVFormatContext* formatCtx, const std::string& mediaPath);
	~AudioInfo() override = default;
};

class FrameExtractorFFMPEG : public IFrameExtractor
{
public:
	explicit FrameExtractorFFMPEG(const std::string& media_path);
	~FrameExtractorFFMPEG() override = default;

	FramePackPtr nextVideo() const override;
	FramePackPtr nextAudio() const override;
	double videoFramerate() const override;
	double audioFramerate() const override;
	int64_t videoFrames() const override;
	int64_t audioFrames() const override;
	bool hasVideo() const override;
	bool hasAudio() const override;

private:
	void setup(const std::string& mediaPath);

	std::unique_ptr<AvDecoder> _videoDecoder;
	std::unique_ptr<AvDecoder> _audioDecoder;
	std::unique_ptr<MediaInfo> _videoInfo;
	std::unique_ptr<MediaInfo> _audioInfo;
};

}

#endif // __OPEN_VIDEO_INTELLIGENCE_FRAME_EXTRACTOR_FFMPEG_H__
