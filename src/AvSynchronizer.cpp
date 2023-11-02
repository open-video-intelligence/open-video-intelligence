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

#include "AvSynchronizer.h"
#include "FramePack.h"
#include "Log.h"

using namespace ovi;

AvSynchronizer::AvSynchronizer(std::shared_ptr<IFrameExtractor> frameExtractor)
	: _frameExtractor(frameExtractor),
	_videoEOF(!_frameExtractor->mediaInfo()->hasVideo()),
	_audioEOF(!_frameExtractor->mediaInfo()->hasAudio()),
	_pts(NO_PTS)
{
}

FramePackPtr AvSynchronizer::getNextVideo()
{
	if (_videoEOF)
		return nullptr;

	return getNext(MEDIA_TYPE_VIDEO, _pts, _videoEOF);
}

std::vector<FramePackPtr> AvSynchronizer::getNextAudio()
{
	std::vector<FramePackPtr> frames;

	if (_audioEOF)
		return frames;

	/* If content has AV, audio frames longer than video are discarded because content is edited based on video frames. */
	if (_videoEOF && _frameExtractor->mediaInfo()->hasVideo())
		return frames;

	while (!_audioEOF) {
		double pts;
		FramePackPtr frame = getNext(MEDIA_TYPE_AUDIO, pts, _audioEOF);
		if (!frame)
			break;
		frames.push_back(std::move(frame));

		if ((_pts == NO_PTS) || ( pts > _pts))
			break;
	};

	return frames;
}

FramePackPtr AvSynchronizer::getNext(MediaType type, double& pts, bool& eof)
{
	FramePackPtr frame;

	if (type == MEDIA_TYPE_VIDEO)
		frame = _frameExtractor->nextVideo();
	else
		frame = _frameExtractor->nextAudio();

	if (frame) {
		LOG_DEBUG("frame num:%d pts:%f", frame->frameNum(), frame->pts());
		pts = frame->pts();
	} else {
		LOG_DEBUG("%s EOF", (type == MEDIA_TYPE_VIDEO) ? "Video" : "Audio");
		eof = true;
	}

	return frame;
}
