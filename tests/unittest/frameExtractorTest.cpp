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

#include "utBase.h"
#include "FrameExtractorFactory.h"
#include "FramePack.h"
#include "FramePackerFactory.h"

class FrameExtractorTest : public UtBase {
protected:
	void SetUp(void) override {
		Start();
	}

	void TearDown(void) override {
		End();
	}
};

TEST_F(FrameExtractorTest, create_check_invalid_parameter_exception)
{
	try {
		FramePackerFactory::create(MEDIA_TYPE_NONE);
	} catch (Exception const& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(FrameExtractorTest, create_check_no_such_file_exception)
{
	try {
		FrameExtractorFactory::create("invalid_media");
	} catch (Exception const& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_NO_SUCH_FILE);
	}
}

TEST_F(FrameExtractorTest, create_check_permission_denied_exception)
{
	try {
		FrameExtractorFactory::create("movie_noaccess.mp4");
	} catch (Exception const& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_PERMISSION_DENIED);
	}
}

TEST_F(FrameExtractorTest, create_check_video_info_return_value)
{
	auto frameExtractor = std::unique_ptr<IFrameExtractor>(FrameExtractorFactory::create(getMediaPath()));
	auto mediaInfo = frameExtractor->mediaInfo();

	ASSERT_TRUE(mediaInfo->hasVideo());
	EXPECT_EQ(mediaInfo->video()->framerate(), 25.0);
	EXPECT_EQ(mediaInfo->video()->frameNum(), 352);
}

TEST_F(FrameExtractorTest, create_check_audio_info_return_value)
{
	auto frameExtractor = std::unique_ptr<IFrameExtractor>(FrameExtractorFactory::create(getMediaPath()));
	auto mediaInfo = frameExtractor->mediaInfo();

	ASSERT_TRUE(mediaInfo->hasAudio());
	EXPECT_EQ(mediaInfo->audio()->framerate(), 43.138951912201449);
	EXPECT_EQ(mediaInfo->audio()->frameNum(), 608);
}

TEST_F(FrameExtractorTest, nextVideo_check_return_value)
{
	auto frameExtractor = std::unique_ptr<IFrameExtractor>(FrameExtractorFactory::create(getMediaPath()));

	FramePackPtr vFrame = frameExtractor->nextVideo();
	EXPECT_TRUE(vFrame);
	EXPECT_TRUE(vFrame->valid());
	EXPECT_EQ(vFrame->type(), MEDIA_TYPE_VIDEO);
}

TEST_F(FrameExtractorTest, nextAudio_check_return_value)
{
	auto frameExtractor = std::unique_ptr<IFrameExtractor>(FrameExtractorFactory::create(getMediaPath()));

	FramePackPtr aFrame = frameExtractor->nextAudio();
	EXPECT_TRUE(aFrame);
	EXPECT_TRUE(aFrame->valid());
	EXPECT_EQ(aFrame->type(), MEDIA_TYPE_AUDIO);
}

TEST_F(FrameExtractorTest, nextVideo_check_until_eof)
{
	auto frameExtractor = std::unique_ptr<IFrameExtractor>(FrameExtractorFactory::create(getMediaPath()));
	int i = 0;

	while (1) {
		FramePackPtr frame = frameExtractor->nextVideo();
		if (!frame)
			break;
		const VideoFramePack* vFrame = dynamic_cast<VideoFramePack*>(frame.get());
		assert(vFrame);

		if (i++ %100 == 0)
			vFrame->dump2File("./", true);
	}
}

TEST_F(FrameExtractorTest, nextAudio_check_until_eof)
{
	auto frameExtractor = std::unique_ptr<IFrameExtractor>(FrameExtractorFactory::create(getMediaPath()));
	int i = 0;

	while (1) {
		FramePackPtr frame = frameExtractor->nextAudio();
		if (!frame)
			break;
		const AudioFramePack* aFrame = dynamic_cast<AudioFramePack*>(frame.get());
		assert(aFrame);

		if (i++ %100 == 0)
			aFrame->dump2File("./", true);
	}
}
