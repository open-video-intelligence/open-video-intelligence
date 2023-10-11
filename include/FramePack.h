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

#ifndef __OPEN_VIDEO_INTELLIGENCE_FRAME_PACK_H__
#define __OPEN_VIDEO_INTELLIGENCE_FRAME_PACK_H__

#include <tuple>
#include <vector>
#include "Types.h"
#include "IFormatConverter.h"

namespace ovi {

using VideoProps = std::tuple<int, int, VideoFormat>;
using AudioProps = std::tuple<int, int, AudioFormat, int>;

using FramePackPtr = std::unique_ptr<FramePack>;

class FramePack
{
public:
	explicit FramePack(MediaType type);
	virtual ~FramePack() = default;

	void assign(const void* buffer, size_t size, int frameNum, double pts, double framerate, int64_t duration = 0);
	void assign(const std::vector<char>& buffer, int frameNum, double pts, double framerate, int64_t duration = 0);

	MediaType type() const { return _type; }
	bool empty() const { return _buffer.empty(); }
	const void* data() const { return (_buffer.data()); }
	size_t size() const { return _buffer.size(); }
	int frameNum() const { return _frameNum; }
	double pts() const { return _pts; }
	double framerate() const { return _framerate; }
	int64_t duration() const { return _duration; }

	virtual FramePackPtr convert(const std::vector<int>& dstFormats) = 0;

	virtual bool valid() const { return (!empty()); }
	virtual void dump2Log(const std::string& tag) const = 0;
	virtual void dump2File(const std::string& path, bool increase = false) const = 0;

protected:
	MediaType _type { MEDIA_TYPE_NONE };
	std::vector<char> _buffer;
	int _frameNum {};
	double _pts {};
	double _framerate {};
	int64_t _duration {};
};


class VideoFramePack : public FramePack
{
public:
	explicit VideoFramePack(const VideoFramePack& ref);
	VideoFramePack(int width, int height, VideoFormat format);

	~VideoFramePack() override = default;

	FramePackPtr convert(const std::vector<int>& dstFormats) override;
	bool valid() const override;
	void dump2Log(const std::string& tag) const override;
	void dump2File(const std::string& path, bool increase = false) const override;

	VideoProps videoProperties() const;

private:
	int _width {};
	int _height {};
	VideoFormat _format { VIDEO_FORMAT_NONE };
	static IFormatConverterPtr _converter;
};

class AudioFramePack : public FramePack
{
public:
	explicit AudioFramePack(const AudioFramePack& ref);
	AudioFramePack(int channels, int samplerate, AudioFormat format, int samples);

	~AudioFramePack() override = default;

	FramePackPtr convert(const std::vector<int>& dstFormats) override;
	bool valid() const override;
	void dump2Log(const std::string& tag) const override;
	void dump2File(const std::string& path, bool increase = false) const override;

	void setChannelLayout(uint64_t channelLayout);
	AudioProps audioProperties() const;
	uint64_t channelLayout() const { return _channelLayout; }

private:
	int _channels {};
	int _samplerate {};
	AudioFormat _format { AUDIO_FORMAT_NONE };
	int _samples {};
	uint64_t _channelLayout {};
	static IFormatConverterPtr _converter;
};


}

#endif // __OPEN_VIDEO_INTELLIGENCE_FRAME_PACK_H__
