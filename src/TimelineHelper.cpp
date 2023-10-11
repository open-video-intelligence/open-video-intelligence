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

#include "TimelineHelper.h"
#include "Exception.h"
#include "Log.h"

using namespace ovi;

TimelineHelper::TimelineHelper(double framerate)
	: _timeline(new otio::Timeline("Timeline", otio::RationalTime(0, framerate))),
	_globalFramerate(framerate)
{
}

void TimelineHelper::makeMediaRef(const std::string& mediaPath,
								double framerate,
								double duration)
{
	try {
		findRef(mediaPath);
	} catch (const Exception& e) {
		refRetainer mediaRef = new otio::ExternalReference(
			mediaPath,
			otio::TimeRange(
				otio::RationalTime(0, framerate),
				otio::RationalTime(duration, framerate)
			)
		);
		_mediaRefs.push_back(mediaRef);
	}
}

void TimelineHelper::appendTrack(const std::string& name,
								MediaType type,
								const std::vector<effectRetainer>& effectList)
{
	trackRetainer track = new otio::Track(name);
	if (type == MEDIA_TYPE_AUDIO)
		track->set_kind(otio::Track::Kind::audio);

	track.value->effects() = effectList;

	_tracks.push_back(track);
}

void TimelineHelper::appendGap(const std::string& trackName, TimeRange range)
{
	try {
		gapRetainer gap = new otio::Gap(
			otio::TimeRange(
				otio::RationalTime(range.startFrameNum, _globalFramerate),
				otio::RationalTime(range.duration, _globalFramerate)
			)
		);

		otio::ErrorStatus err;
		if (!findTrack(trackName)->append_child(gap, &err))
			throw Exception(OVI_ERROR_INVALID_OPERATION, err.details);

	} catch (const Exception& e) {
		LOG_ERROR("Error: %s", e.what());
		throw Exception(e.error(), "appendGap failed");
	}
}

void TimelineHelper::appendClip(const std::string& trackName,
								const std::string& clipName,
								TimeRange range,
								const std::string& mediaPath,
								const std::vector<effectRetainer>& effectList)
{
	try {
		auto ref = findRef(mediaPath);
		double rate = ref->available_range()->start_time().rate();
		clipRetainer clip = new otio::Clip(
			clipName,
			ref,
			otio::TimeRange(
				otio::RationalTime(range.startFrameNum, rate),
				otio::RationalTime(range.duration, rate)
			)
		);

		clip.value->effects() = effectList;

		otio::ErrorStatus err;
		if (!findTrack(trackName)->append_child(clip, &err))
			throw Exception(OVI_ERROR_INVALID_OPERATION, err.details);

	} catch (const Exception& e) {
		LOG_ERROR("Error: %s", e.what());
		throw Exception(e.error(), "appendClip failed");
	}
}

effectRetainer TimelineHelper::makeEffect(const std::string& name,
										const std::string& effect,
										otio::AnyDictionary metadata)
{
	return new otio::Effect(name, effect, metadata);
}

trackRetainer TimelineHelper::findTrack(const std::string& name) const
{
	auto iter = std::find_if(_tracks.begin(), _tracks.end(),
							[&](auto track) -> bool { return track->name() == name; });
	if (iter == _tracks.end())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "No track");

	return *iter;
}

refRetainer TimelineHelper::findRef(const std::string& path) const
{
	auto iter = std::find_if(_mediaRefs.begin(), _mediaRefs.end(),
							[&](auto ref) -> bool { return ref->target_url() == path; });
	if (iter == _mediaRefs.end())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "No mediaRef");

	return *iter;
}

timelineRetainer const& TimelineHelper::getTimeline()
{
	for (const auto& track : _tracks) {
		otio::TimeRange child_range = track->trimmed_range();
		track->set_source_range(child_range);

		otio::ErrorStatus err;
		if (!_timeline->tracks()->append_child(track, &err))
			throw Exception(OVI_ERROR_INVALID_OPERATION, err.details);
	}

	return _timeline;
}
