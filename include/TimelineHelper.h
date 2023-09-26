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

#ifndef __OPEN_VIDEO_INTELLIGENCE_TIMELINE_HELPER_H__
#define __OPEN_VIDEO_INTELLIGENCE_TIMELINE_HELPER_H__

#include <iostream>
#include <vector>
#include "opentimelineio/clip.h"
#include "opentimelineio/track.h"
#include "opentimelineio/typeRegistry.h"
#include "opentimelineio/serialization.h"
#include "opentimelineio/deserialization.h"
#include "opentimelineio/timeline.h"
#include "opentimelineio/externalReference.h"
#include "opentimelineio/gap.h"
#include "opentimelineio/effect.h"
#include "opentimelineio/transition.h"
#include "opentimelineio/anyDictionary.h"

#include "Types.h"

namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;
using trackRetainer = otio::SerializableObject::Retainer<otio::Track>;
using effectRetainer = otio::SerializableObject::Retainer<otio::Effect>;
using refRetainer = otio::SerializableObject::Retainer<otio::ExternalReference>;
using timelineRetainer = otio::SerializableObject::Retainer<otio::Timeline>;
using clipRetainer = otio::SerializableObject::Retainer<otio::Clip>;
using gapRetainer = otio::SerializableObject::Retainer<otio::Gap>;

namespace ovi {

struct TimeRange {
	double startFrameNum {};
	double duration {};
};

class TimelineHelper
{
public:
	explicit TimelineHelper(double framerate);
	~TimelineHelper() = default;

	void makeMediaRef(const std::string& mediaPath,
					double framerate,
					double duration);

	void appendTrack(const std::string& name,
					MediaType type = MEDIA_TYPE_VIDEO,
					const std::vector<effectRetainer>& effectList = {});

	void appendGap(const std::string& trackName, TimeRange range);

	void appendClip(const std::string& trackName,
					const std::string& clipName,
					TimeRange range,
					const std::string& mediaPath,
					const std::vector<effectRetainer>& effectList = {});

	effectRetainer makeEffect(const std::string& name,
							const std::string& effect,
							otio::AnyDictionary metadata);

	timelineRetainer const& getTimeline();

private:
	trackRetainer findTrack(const std::string& name) const;
	refRetainer findRef(const std::string& path) const;

	timelineRetainer _timeline;
	std::vector<trackRetainer> _tracks;
	std::vector<refRetainer> _mediaRefs;

	double _globalFramerate {};
};

} // ovi

#endif // __OPEN_VIDEO_INTELLIGENCE_TIMELINE_HELPER_H__
