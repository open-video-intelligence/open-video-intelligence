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

#include <tuple>
#include <vector>

#include "utBase.h"
#include "FormatConverterFactory.h"
#include "FramePack.h"


class FormatConvertTest : public UtBase {
protected:
	// cppcheck-suppress unusedFunction
	void SetUp(void) override {
		Start();
	}

	// cppcheck-suppress unusedFunction
	void TearDown(void) override {
		End();
	}

	void initVideoSource();
	void initAudioSource();

	bool compareAnswer(const FramePack* frame, const std::string& path);
	bool checkVideoFrame(const FramePack* frame);
	bool checkAudioFrame(const FramePack* frame);

	FramePackPtr audioFrame;
	FramePackPtr videoFrame;

	const std::string videoRaw = "video_rgb_360x360.raw";
	const std::string audioRaw = "audio_fltp_2_44100_1024.raw";
};

void FormatConvertTest::initVideoSource()
{
	/* if the source is changed, please update below according to the source */
	VideoFramePack* newFrame = new VideoFramePack(360, 360, VIDEO_FORMAT_RGB24);

	newFrame->assign(readFile(videoRaw), 1, 0, 1, 1);
	videoFrame = FramePackPtr(newFrame);
}

void FormatConvertTest::initAudioSource()
{
	/* if the source is changed, please update below according to the source */
	AudioFramePack* newFrame = new AudioFramePack(2, 44100, AUDIO_FORMAT_FLTP, 1024);

	newFrame->assign(readFile(audioRaw), 1, 0, 1, 1);
	newFrame->setChannelLayout((uint64_t)0x11/*stereo of ffmpeg*/);
	audioFrame = FramePackPtr(newFrame);
}

bool FormatConvertTest::compareAnswer(const FramePack* frame, const std::string& path)
{
	bool result = true;

	std::vector<char> buffer = readFile(path);

	if (buffer.size() != frame->size()) {
		std::cout << "invalid size [" << buffer.size() << " : " << frame->size() << "]" << std::endl;
		result = false;
	}

	/* FIXME : DEREF_OF_NULL.RET-DEREF_OF_NULL.RET.STAT ? */
	if (!buffer.empty() && 0 != memcmp(buffer.data(), frame->data(), frame->size())) {
		std::cout << "invalid data" << std::endl;
		result = false;
	}

	return result;
}

bool FormatConvertTest::checkVideoFrame(const FramePack* frame)
{
	std::stringstream s;
	const VideoFramePack* vFrame = dynamic_cast<const VideoFramePack*>(frame);
	assert(vFrame);

	if (!frame) {
		std::cout << "invalid" << std::endl;
		return false;
	}

	if (frame->empty()) {
		std::cout << "invalid" << std::endl;
		return false;
	}

	if (frame->type() != MEDIA_TYPE_VIDEO) {
		std::cout << "not video" << std::endl;
		return false;
	}

	auto [ width, height, format ] = vFrame->videoProperties();
	s << "video_" << width << "x" << height << "_" << format << ".raw";

	return compareAnswer(frame, s.str());
}

bool FormatConvertTest::checkAudioFrame(const FramePack* frame)
{
	std::stringstream s;
	const AudioFramePack* aFrame = dynamic_cast<const AudioFramePack*>(frame);
	assert(aFrame);

	if (!frame) {
		std::cout << "invalid" << std::endl;
		return false;
	}

	if (frame->empty()) {
		std::cout << "invalid" << std::endl;
		return false;
	}

	if (frame->type() != MEDIA_TYPE_AUDIO) {
		std::cout << "not video" << std::endl;
		return false;
	}

	auto [ channels, samplerate, format, samples ] = aFrame->audioProperties();
	s << "audio_" << channels << "_" << samplerate << "_" << samples << "_" << format << ".raw";

	return compareAnswer(frame, s.str());
}

TEST_F(FormatConvertTest, convert_video)
{
	initVideoSource();

	for (int format = (int)(VIDEO_FORMAT_NONE + 1); format < (int)VIDEO_FORMAT_MAX; format++) {
		try {
			const std::vector<int> formats = { format };
			FramePackPtr result = videoFrame->convert(formats);
			EXPECT_TRUE(checkVideoFrame(result.get()));
		} catch (const Exception& e) {
			std::cout << "Error: " << e.what() << std::endl;
			EXPECT_TRUE(false);
		}
	}
}

TEST_F(FormatConvertTest, convert_audio)
{
	initAudioSource();

	for (int format = (int)(AUDIO_FORMAT_NONE + 1); format < (int)AUDIO_FORMAT_MAX; format++) {
		try {
			const std::vector<int> formats = { format };
			FramePackPtr result = audioFrame->convert(formats);
			EXPECT_TRUE(checkAudioFrame(result.get()));
		} catch (const Exception& e) {
			std::cout << "Error: " << e.what() << std::endl;
			EXPECT_TRUE(false);
		}
	}
}

TEST_F(FormatConvertTest, convert_invalid_argument_video)
{
	// TODO: using TestWithParm
	std::vector<std::tuple<int, int, VideoFormat, std::vector<int>>> badVideo {
		{ 0, 100, VIDEO_FORMAT_RGB24, { VIDEO_FORMAT_BGR24 } },
		{ 100, 0, VIDEO_FORMAT_RGB24, { VIDEO_FORMAT_BGR24 } },
		{ 100, 100, VIDEO_FORMAT_NONE, { VIDEO_FORMAT_BGR24 } },
		{ 100, 100, VIDEO_FORMAT_RGB24, { VIDEO_FORMAT_NONE, VIDEO_FORMAT_MAX } }
	};

	initVideoSource();

	for (const auto& [ width, height, format, destFormat ] : badVideo) {
		try {
			auto frame = std::make_unique<VideoFramePack>(width, height, format);

			FramePackPtr result = frame->convert(destFormat);

		} catch (const Exception& e) {
			EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
		}
	}
}

TEST_F(FormatConvertTest, convert_invalid_argument_audio)
{
	std::vector<std::tuple<int, int, AudioFormat, int, int, std::vector<int>>> badAudio {
		{ 0, 44100, AUDIO_FORMAT_FLTP, 1024, 0x11, { AUDIO_FORMAT_S32P } },
		{ 2, 0, AUDIO_FORMAT_FLTP, 1024, 0x11, { AUDIO_FORMAT_S32P } },
		{ 2, 44100, AUDIO_FORMAT_NONE, 1024, 0x11, { AUDIO_FORMAT_S32P } },
		{ 2, 44100, AUDIO_FORMAT_FLTP, 0, 0x11, { AUDIO_FORMAT_S32P } },
		{ 2, 44100, AUDIO_FORMAT_FLTP, 1024, 0, { AUDIO_FORMAT_S32P } },
		{ 2, 44100, AUDIO_FORMAT_FLTP, 1024, 0x11, { AUDIO_FORMAT_NONE, AUDIO_FORMAT_MAX } }
	};

	initAudioSource();
	IFormatConverterPtr converter = FormatConverterFactory::create(MEDIA_TYPE_AUDIO);

	for (const auto& [ channels, samplerate, format, samples, channelLayout, destFormat ] : badAudio) {
		try {
			auto frame = std::make_unique<AudioFramePack>(channels, samplerate, format, samples);
			frame->setChannelLayout(static_cast<uint64_t>(channelLayout));

			FramePackPtr result = frame->convert(destFormat);

		} catch (const Exception& e) {
			EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
		}
	}
}

TEST_F(FormatConvertTest, create_with_invalid_type)
{
	try {
		IFormatConverterPtr converter = FormatConverterFactory::create(MEDIA_TYPE_NONE);
		EXPECT_TRUE(false);
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(FormatConvertTest, convert_video_null_frame)
{
	try {
		IFormatConverterPtr converter = FormatConverterFactory::create(MEDIA_TYPE_VIDEO);
		converter->convert(nullptr, { VIDEO_FORMAT_BGR24 });
		EXPECT_TRUE(false);
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}

TEST_F(FormatConvertTest, convert_audio_null_frame)
{
	try {
		IFormatConverterPtr converter = FormatConverterFactory::create(MEDIA_TYPE_AUDIO);
		converter->convert(nullptr, { AUDIO_FORMAT_S16 });
		EXPECT_TRUE(false);
	} catch (const Exception& e) {
		std::cout << "[EXPECTED] Error: " << e.what() << std::endl;
		EXPECT_EQ(e.error(), OVI_ERROR_INVALID_PARAMETER);
	}
}
