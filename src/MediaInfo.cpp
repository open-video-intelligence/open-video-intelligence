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

#include "Log.h"
#include "MediaInfo.h"

using namespace ovi;

BaseInfo::BaseInfo(MediaType type)
	: _type(type)
{
}

BaseInfo::BaseInfo(MediaType type, BaseProperties props)
	: _type(type)
{
	setProperties(props);
}

void BaseInfo::setProperties(BaseProperties props)
{
	auto [framerate, bitRate, frameNum] = props;

	_framerate = framerate;
	_bitRate = bitRate;
	_frameNum = frameNum;
}

BaseProperties BaseInfo::properties() const
{
	return std::make_tuple(_framerate, _bitRate, _frameNum);
}

VideoInfo::VideoInfo()
	: BaseInfo(MEDIA_TYPE_VIDEO)
{
}

VideoInfo::VideoInfo(VideoProperties props)
	: BaseInfo(MEDIA_TYPE_VIDEO)
{
	setProperties(props);
}

VideoInfo::VideoInfo(const VideoInfo& ref)
	: BaseInfo(MEDIA_TYPE_VIDEO)
{
	setProperties(ref.properties());
}

void VideoInfo::setProperties(VideoProperties props)
{
	auto [baseProps, width, height] = props;

	BaseInfo::setProperties(baseProps);
	_width = width;
	_height = height;
}

VideoProperties VideoInfo::properties() const
{
	return std::make_tuple(BaseInfo::properties(), _width, _height);
}

AudioInfo::AudioInfo()
	: BaseInfo(MEDIA_TYPE_AUDIO)
{
}

AudioInfo::AudioInfo(AudioProperties props)
	: BaseInfo(MEDIA_TYPE_AUDIO)
{
	setProperties(props);
}

AudioInfo::AudioInfo(const AudioInfo& ref)
	: BaseInfo(MEDIA_TYPE_AUDIO)
{
	setProperties(ref.properties());
}

void AudioInfo::setProperties(AudioProperties props)
{
	auto [baseProps, samplePerSec, bitPerSample] = props;

	BaseInfo::setProperties(baseProps);
	_samplePerSec = samplePerSec;
	_bitPerSample = bitPerSample;
}

AudioProperties AudioInfo::properties() const
{
	return std::make_tuple(BaseInfo::properties(), _samplePerSec, _bitPerSample);
}

MediaInfo::MediaInfo(const MediaInfo& ref)
	: _mediaPath(ref.mediaPath())
{
	if (ref.hasVideo())
		_video = std::make_shared<VideoInfo>(ref.video()->properties());
	if (ref.hasAudio())
		_audio = std::make_shared<AudioInfo>(ref.audio()->properties());
}