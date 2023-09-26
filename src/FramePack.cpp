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

#include <sstream>
#include <fstream>
#include <cstring>

#include "FramePack.h"
#include "Log.h"
#include "Exception.h"
#include "FormatConverterFactory.h"

using namespace ovi;

FramePack::FramePack(MediaType type)
	: _type(type)
{
}

void FramePack::assign(const void* buffer, size_t size, int frameNum, double pts, double framerate, int64_t duration)
{
	if (size == 0)
		return;

	auto ptr = static_cast<char*>(const_cast<void*>(buffer));

	_buffer.assign(ptr, ptr + size);
	_frameNum = frameNum;
	_pts = pts;
	_framerate = framerate;
	_duration = duration;
}

void FramePack::assign(const std::vector<char>& buffer, int frameNum, double pts, double framerate, int64_t duration)
{
	if (buffer.empty())
		return;

	_buffer = buffer;
	_frameNum = frameNum;
	_pts = pts;
	_framerate = framerate;
	_duration = duration;
}

IFormatConverterPtr VideoFramePack::_converter = nullptr;

VideoFramePack::VideoFramePack(int width, int height, VideoFormat format)
	: FramePack(MEDIA_TYPE_VIDEO), _width(width), _height(height), _format(format)
{
	if (!_converter)
		_converter = FormatConverterFactory::create(MEDIA_TYPE_VIDEO);
}

VideoFramePack::VideoFramePack(const VideoFramePack &ref)
	: FramePack(MEDIA_TYPE_VIDEO)
{
	std::tie(_width, _height, _format) = ref.videoProperties();
	assign((void*)(ref.data()), ref.size(), ref.frameNum(), ref.pts(), ref.framerate(), ref.duration());
}

FramePackPtr VideoFramePack::convert(const std::vector<int>& dstFormats)
{
	if (!_converter)
		throw Exception(OVI_ERROR_INVALID_OPERATION,"The format can't be converted due to invalid converter.");
	return _converter->convert(this, dstFormats);
}

bool VideoFramePack::valid() const
{
	if (_width < 0 || _height < 0)
		return false;

	return FramePack::valid();
}
// LCOV_EXCL_START
void VideoFramePack::dump2Log(const std::string& tag) const
{
	std::stringstream s;

	s << "===== Dump(" << tag << ") =====\n";
	s << " type:" << std::to_string(_type) << ", format:" << _format;
	s << ", width:" << _width << ", height:" << _height;

	LOG_DEBUG("%s", s.str().c_str());
}
// LCOV_EXCL_STOP
void VideoFramePack::dump2File(const std::string& path, bool incremental) const
{
	std::stringstream s;
	s << path << "video_" << _width << "x" << _height << "_" << _format;

	if (incremental) {
		static unsigned long count = 0;
		s << "_" << count++;
	}
	s << ".raw";

	std::ofstream fileWrite(s.str());
	if (fileWrite.fail())
		LOG_ERROR("error opening the file %s to write)", s.str().c_str());
	else
		fileWrite.write(_buffer.data(), static_cast<std::streamsize>(_buffer.size()));

}

VideoProps VideoFramePack::videoProperties() const
{
	return std::make_tuple(_width, _height, _format);
}

IFormatConverterPtr AudioFramePack::_converter = nullptr;

AudioFramePack::AudioFramePack(int channels, int samplerate, AudioFormat format, int samples)
	: FramePack(MEDIA_TYPE_AUDIO), _channels(channels), _samplerate(samplerate), _format(format), _samples(samples)
{
	if (!_converter)
		_converter = FormatConverterFactory::create(MEDIA_TYPE_AUDIO);
}

AudioFramePack::AudioFramePack(const AudioFramePack &ref)
	: FramePack(MEDIA_TYPE_AUDIO)
{
	std::tie(_channels, _samplerate, _format, _samples) = ref.audioProperties();
	assign((void*)(ref.data()), ref.size(), ref.frameNum(), ref.pts(), ref.framerate(), ref.duration());
	setChannelLayout(ref.channelLayout());
}

FramePackPtr AudioFramePack::convert(const std::vector<int>& dstFormats)
{
	if (!_converter)
		throw Exception(OVI_ERROR_INVALID_OPERATION,"The format can't be converted due to invalid converter.");
	return _converter->convert(this, dstFormats);
}

bool AudioFramePack::valid() const
{
	if (_channels < 0 || _samplerate < 0 || _samples < 0)
		return false;

	return FramePack::valid();
}

void AudioFramePack::setChannelLayout(uint64_t channelLayout)
{
	_channelLayout = channelLayout;
}

AudioProps AudioFramePack::audioProperties() const
{
	return std::make_tuple(_channels, _samplerate, _format, _samples);
}
// LCOV_EXCL_START
void AudioFramePack::dump2Log(const std::string& tag) const
{
	std::stringstream s;

	s << "===== Dump(" << tag << ") =====\n";
	s << " type:" << std::to_string(_type) << ", format:" << _format;
	s << ", channels:" << _channels << ", samplerate:" << _samplerate;
	s << ", samples:" << _samples << ", channelLayout:" << _channelLayout;

	LOG_DEBUG("%s", s.str().c_str());
}
// LCOV_EXCL_STOP
void AudioFramePack::dump2File(const std::string& path, bool incremental) const
{
	std::stringstream s;
	s << path << "audio_" << _channels << "_" << _samplerate;
	s << "_" << _format << "_" << _samples;

	if (incremental) {
		static unsigned long count = 0;
		s << "_" << count++;
	}
	s << ".raw";

	std::ofstream fileWrite(s.str());
	if (fileWrite.fail())
		LOG_ERROR("error opening the file %s to write)", s.str().c_str());
	else
		fileWrite.write(_buffer.data(), static_cast<std::streamsize>(_buffer.size()));
}
