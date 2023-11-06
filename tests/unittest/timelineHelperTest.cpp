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

#include <any>

#include "utBase.h"
#include "TimelineHelper.h"


class TimelineHelperTest : public UtBase {
protected:

	TimelineHelper tl = TimelineHelper(24);
	double framerate() { return 24; }
	double duration() { return 19920; }
};

TEST_F(TimelineHelperTest, appendTrack_check_video)
{
	try {
		tl.appendTrack("TRACK");
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(TimelineHelperTest, appendTrack_check_audio)
{
	try {
		tl.appendTrack("TRACK", MEDIA_TYPE_AUDIO);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(TimelineHelperTest, appendTrack_check_audio_and_effect)
{
	auto effect = tl.makeEffect("EFFECT", "blur", {{ "sigma_x", "10.0" }, { "sigma_y", "0.0"}});

	try {
		tl.appendTrack("TRACK", MEDIA_TYPE_AUDIO, { effect });
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(TimelineHelperTest, appendClip_check_video_track)
{
	TimeRange tr = {10, 10};
	tl.appendTrack("TRACK", MEDIA_TYPE_VIDEO);
	tl.makeMediaRef(getMediaPath(), framerate(), duration());

	try {
		tl.appendClip("TRACK", "CLIP", tr, getMediaPath());
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(TimelineHelperTest, appendClip_check_video_track_and_effect)
{
	TimeRange tr = {10, 10};
	tl.appendTrack("TRACK", MEDIA_TYPE_VIDEO);
	tl.makeMediaRef(getMediaPath(), framerate(), duration());
	auto effect = tl.makeEffect("EFFECT", "blur", {{ "sigma_x", "10.0" }, { "sigma_y", "0.0"}});

	try {
		tl.appendClip("TRACK", "CLIP", tr, getMediaPath(), { effect });
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(TimelineHelperTest, appendClip_check_audio_track)
{
	TimeRange tr = {10, 10};
	tl.appendTrack("TRACK", MEDIA_TYPE_AUDIO);
	tl.makeMediaRef(getMediaPath(), framerate(), duration());

	try {
		tl.appendClip("TRACK", "CLIP", tr, getMediaPath());
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(TimelineHelperTest, appendClip_check_invalid_parameter_exception)
{
	TimeRange tr = {10, 10};
	tl.makeMediaRef(getMediaPath(), framerate(), duration());

	try {
		tl.appendClip("TRACK", "CLIP", tr, getMediaPath());
	} catch (const Exception& e) {
		EXPECT_EQ(OVI_ERROR_INVALID_PARAMETER, e.error());
	}
}

TEST_F(TimelineHelperTest, appendGap_check_video_track)
{
	TimeRange tr = {10, 10};
	tl.appendTrack("TRACK", MEDIA_TYPE_VIDEO);
	try {
		tl.appendGap("TRACK", tr);
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(TimelineHelperTest, appendGap_check_invalid_parameter_exception)
{
	TimeRange tr = {10, 10};

	try {
		tl.appendGap("TRACK", tr);
	} catch (const Exception& e) {
		EXPECT_EQ(OVI_ERROR_INVALID_PARAMETER, e.error());
	}
}

TEST_F(TimelineHelperTest, makeMediaRef_check_exception)
{
	try {
		tl.makeMediaRef(getMediaPath(), framerate(), duration());
	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(TimelineHelperTest, makeEffect_check_return_value)
{
	try {
		auto effect = tl.makeEffect("EFFECT", "blur", {{ "sigma_x", "10.0" }, { "sigma_y", "0.0"}});

		EXPECT_EQ(effect->name(), "EFFECT");
		EXPECT_EQ(effect->effect_name(), "blur");

		auto dic = effect->metadata();
		EXPECT_EQ(std::any_cast<const char*>(dic["sigma_x"]), "10.0");
		EXPECT_EQ(std::any_cast<const char*>(dic["sigma_y"]), "0.0");

	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}

TEST_F(TimelineHelperTest, getTimeline_check_return_value)
{
	TimeRange tr = {10, 10};
	tl.appendTrack("TRACK", MEDIA_TYPE_VIDEO);
	tl.makeMediaRef(getMediaPath(), framerate(), duration());

	try {
		tl.appendClip("TRACK", "CLIP", tr, getMediaPath());
		auto res = tl.getTimeline();

		const std::vector<clipRetainer> clips = res->find_clips();

		for (const auto& clip : clips) {
			EXPECT_NE(clip->name().c_str(), nullptr);
			auto trimmed_range = clip->trimmed_range();
			EXPECT_DOUBLE_EQ(trimmed_range.start_time().value(), 10.0);
			EXPECT_DOUBLE_EQ(trimmed_range.start_time().rate(), 24.0);
			EXPECT_DOUBLE_EQ(trimmed_range.duration().value(), 10.0);
			EXPECT_DOUBLE_EQ(trimmed_range.duration().rate(), 24.0);
		}

	} catch (const Exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		EXPECT_TRUE(false);
	}
}
