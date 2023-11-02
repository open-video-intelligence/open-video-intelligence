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

#ifndef __OPEN_VIDEO_INTELLIGENCE_MEDIA_INFO_FFMPEG_H__
#define __OPEN_VIDEO_INTELLIGENCE_MEDIA_INFO_FFMPEG_H__

#include <libavformat/avformat.h>

#include "MediaInfo.h"

namespace ovi {

class FFMPEGInfo
{
public:
	FFMPEGInfo(AVStream* stream, int streamId, const std::string& mediaPath)
		: _stream(stream), _streamId(streamId), _mediaPath(mediaPath) {}

	int streamId() const { return _streamId; }
	bool success() const { return _success; }

protected:
	AVStream* _stream {};
	int _streamId {};
	std::string _mediaPath;

	bool _success {};
};

class VideoInfoFFMPEG : public VideoInfo, public FFMPEGInfo
{
public:
	VideoInfoFFMPEG(AVStream* stream, int streamId, const std::string& mediaPath)
		: VideoInfo(), FFMPEGInfo(stream, streamId, mediaPath)
	{
		extract();
	}

	void extract();
};

class AudioInfoFFMPEG : public AudioInfo, public FFMPEGInfo
{
public:
	AudioInfoFFMPEG(AVStream* stream, int streamId, const std::string& mediaPath)
		: AudioInfo(), FFMPEGInfo(stream, streamId, mediaPath)
	{
		extract();
	}

	void extract();
};

class MediaInfoFFMPEG : public MediaInfo
{
public:
	MediaInfoFFMPEG(const std::string& mediaPath)
		: MediaInfo(mediaPath)
	{
		extract();
	}

	void extract() override;

	int videoStreamId() const { return _videoStreamId; }
	int audioStreamId() const { return _audioStreamId; }

protected:
	int _videoStreamId { -1 };
	int _audioStreamId { -1 };
};

}

#endif /* __OPEN_VIDEO_INTELLIGENCE_MEDIA_INFO_FFMPEG_H__ */