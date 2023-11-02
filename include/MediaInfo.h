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

#ifndef __OPEN_VIDEO_INTELLIGENCE_MEDIA_INFO_H__
#define __OPEN_VIDEO_INTELLIGENCE_MEDIA_INFO_H__

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>

#include "Types.h"

namespace ovi {

using BaseProperties = std::tuple<double, int64_t, int64_t>;
using VideoProperties = std::tuple<BaseProperties, int, int>;
using AudioProperties = std::tuple<BaseProperties, int, int>;

class MediaInfo;
class VideoInfo;
class AudioInfo;
using MediaInfoPtr = std::shared_ptr<MediaInfo>;
using VideoInfoPtr = std::shared_ptr<VideoInfo>;
using AudioInfoPtr = std::shared_ptr<AudioInfo>;

class BaseInfo
{
public:
	explicit BaseInfo(MediaType type);
	explicit BaseInfo(MediaType type, BaseProperties props);
	virtual ~BaseInfo() = default;

	MediaType type() const { return _type; }
	double framerate() const { return _framerate; }
	int64_t bitRate() const { return _bitRate; }
	int64_t frameNum() const { return _frameNum; }

protected:
	void setProperties(BaseProperties props);
	BaseProperties properties() const;

private:
	MediaType _type { MEDIA_TYPE_NONE };
	double _framerate {};
	int64_t _bitRate {};
	int64_t _frameNum {};
};

class VideoInfo : public BaseInfo
{
public:
	VideoInfo();
	VideoInfo(VideoProperties props);
	VideoInfo(const VideoInfo& ref);
	~VideoInfo() = default;

	void setProperties(VideoProperties props);
	VideoProperties properties() const;

private:
	int _width {};
	int _height {};
};

class AudioInfo : public BaseInfo
{
public:
	AudioInfo();
	AudioInfo(AudioProperties props);
	AudioInfo(const AudioInfo& ref);
	~AudioInfo() = default;

	void setProperties(AudioProperties props);
	AudioProperties properties() const;

private:
	int _samplePerSec {};
	int _bitPerSample {};
};

class MediaInfo
{
public:
	MediaInfo(const std::string& mediaPath)
		: _mediaPath(mediaPath) {}
	MediaInfo(const MediaInfo& ref);
	~MediaInfo() = default;

	bool hasVideo() const { return _video != nullptr; }
	bool hasAudio() const { return _audio != nullptr; }

	std::string mediaPath() const { return _mediaPath; }
	VideoInfoPtr video() const { return _video; }
	AudioInfoPtr audio() const { return _audio; }

	virtual void extract() = 0;

protected:
	std::string _mediaPath;
	VideoInfoPtr _video;
	AudioInfoPtr _audio;
};

}

#endif /* __OPEN_VIDEO_INTELLIGENCE_MEDIA_INFO_H__ */
