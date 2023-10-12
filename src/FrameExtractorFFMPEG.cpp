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

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

#include "FrameExtractorFFMPEG.h"
#include "Log.h"
#include "Exception.h"

#include <unistd.h>
#include <optional>
#include <utility>

using namespace ovi;

struct videoInfo {
	unsigned int stream_id {};
	int codec_id {};
	int width {};
	int height {};
	double fps {};
	int64_t bitRate {};
	int64_t frames {};
};

struct audioInfo {
	unsigned int stream_id {};
	int codec_id {};
	int64_t bitRate {};
	int samplePerSec {};
	int bitPerSample {};
	int64_t frames {};
	double fps {};
};

static void __printFFmpegErrorStr(std::string function, int err)
{
	char errorStr[AV_ERROR_MAX_STRING_SIZE] = {0};
	av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, err);
	LOG_ERROR("failed to %s. err:%d", function.c_str(), errorStr);
}

static AVFormatContext* __openFFmpeg(const std::string& _path)
{
	AVFormatContext* _formatCtx = nullptr;

	int ret = avformat_open_input(&_formatCtx, _path.c_str(), nullptr, nullptr);
	if (ret < 0) {
		__printFFmpegErrorStr("avformat_open_input()", ret);
		if (ret == AVERROR_INVALIDDATA)
			throw Exception(OVI_ERROR_NOT_SUPPORTED_MEDIA, "failed to avformat_open_input");

		// FIXME: what about other error?
	} else {
		LOG_DEBUG("Success __openFFmpeg");
	}

	if (avformat_find_stream_info(_formatCtx, NULL) < 0)
		throw Exception(OVI_ERROR_INVALID_OPERATION, "failed to avformat_find_stream_info");

	return _formatCtx;
}

static int64_t __calculateTotalFrames(std::unique_ptr<AvDecoder> decoder)
{
	while (1) {
		AVFrame* frame = decoder->decode();
		if (!frame)
			break;

		av_frame_free(&frame);
	}

	return static_cast<int64_t>(decoder->frameNum());
}

static std::optional<videoInfo> __getVideoInfo(AVFormatContext* _formatCtx, const std::string& mediaPath)
{
	for (unsigned int i = 0; i < _formatCtx->nb_streams; i++) {
		if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (_formatCtx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
				LOG_DEBUG("it's an attached picture(thumbnail)");
				return std::nullopt;
			}

			AVCodecParameters* pVideoCodecPar = _formatCtx->streams[i]->codecpar;
			if (!pVideoCodecPar)
				return std::nullopt;

			int64_t totalFrames = _formatCtx->streams[i]->nb_frames;
			if (totalFrames == 0) {
				totalFrames = __calculateTotalFrames(AvDecoderFactory::createVideoDecoder(i, mediaPath));
				if (totalFrames == 0) {
					LOG_ERROR("failed to get total frame count");
					return std::nullopt;
				}
			}

			LOG_DEBUG("stream_id:%d codec:[%x]%s width:%d height:%d fps:%f bitRate:%" PRId64 " frames:%" PRId64,
				i, pVideoCodecPar->codec_id, avcodec_get_name(pVideoCodecPar->codec_id),
				pVideoCodecPar->width, pVideoCodecPar->height,
				av_q2d(_formatCtx->streams[i]->r_frame_rate), pVideoCodecPar->bit_rate,
				totalFrames);

			return videoInfo {
				.stream_id = i,
				.codec_id = pVideoCodecPar->codec_id,
				.width = pVideoCodecPar->width,
				.height = pVideoCodecPar->height,
				.fps = av_q2d(_formatCtx->streams[i]->r_frame_rate),
				.bitRate = pVideoCodecPar->bit_rate,
				.frames = totalFrames
			};
		}
	}

	return std::nullopt;
}

static std::optional<audioInfo> __getAudioInfo(AVFormatContext* _formatCtx, const std::string& mediaPath)
{
	for (unsigned int i = 0; i < _formatCtx->nb_streams; i++) {
		if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			AVCodecParameters* pAudioCodecPar = _formatCtx->streams[i]->codecpar;
			if (!pAudioCodecPar)
				return std::nullopt;

			int64_t duration = av_rescale_q(_formatCtx->streams[i]->duration,
											_formatCtx->streams[i]->time_base,
											AV_TIME_BASE_Q); //milliseconds

			int64_t totalFrames = _formatCtx->streams[i]->nb_frames;
			if (totalFrames == 0) {
				totalFrames = __calculateTotalFrames(AvDecoderFactory::createAudioDecoder(i, mediaPath));
				if (totalFrames == 0) {
					LOG_ERROR("failed to get total frame count");
					return std::nullopt;
				}
			}

			double fps = totalFrames / ((double)duration / AV_TIME_BASE);

			LOG_DEBUG("stream_id:%d codec:[%x]%s bitRate:%" PRId64 " samplePerSec:%d bitPerSample:%d frames:%" PRId64 " fps:%f",
				i, pAudioCodecPar->codec_id, avcodec_get_name(pAudioCodecPar->codec_id),
				pAudioCodecPar->bit_rate, pAudioCodecPar->sample_rate, pAudioCodecPar->bits_per_coded_sample,
				totalFrames, fps);

			return audioInfo {
				.stream_id = i,
				.codec_id = pAudioCodecPar->codec_id,
				.bitRate = pAudioCodecPar->bit_rate,
				.samplePerSec = pAudioCodecPar->sample_rate,
				.bitPerSample = pAudioCodecPar->bits_per_coded_sample,
				.frames = totalFrames,
				.fps = fps
			};
		}
	}

	return std::nullopt;
}

AvDecoder::AvDecoder(AVMediaType mediaType, int streamId, std::string mediaPath,
					IFramePackerPtr packer)
	: _formatCtx(__openFFmpeg(mediaPath)), _mediaType(mediaType),
	_streamId(streamId), _eof(false), _packer(std::move(packer))
{
	ready();
}

AvDecoder::~AvDecoder()
{
	if (_codecCtx)
		avcodec_free_context(&_codecCtx);

	if (_formatCtx)
		avformat_close_input(&_formatCtx);
}

AVFrame* AvDecoder::decode()
{
	int ret = 0;
	AVPacket* pkt = nullptr;
	AVFrame* frame = nullptr;

	if (!_codecCtx) {
		LOG_ERROR("[decode] invalid pCodecCtx");
		return nullptr;
	}

	do {
		pkt = readFrame();
		if (!pkt)
			break;

		ret = avcodec_send_packet(_codecCtx, pkt);
		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
			LOG_ERROR("failed to avcodec_send_packet()");
			break;
		}

		av_packet_free(&pkt);

		frame = av_frame_alloc();
		if (!frame) {
			LOG_ERROR("failed to av_frame_alloc()");
			break;
		}

		ret = avcodec_receive_frame(_codecCtx, frame);
		switch (ret) {
		case 0:
			_frameNum = _codecCtx->frame_number;
			return frame;
		case AVERROR(EAGAIN):
			LOG_INFO("AVERROR(EAGAIN)");
			break;
		case AVERROR_EOF:
			LOG_INFO("AVERROR_EOF");
			break;
		default:
			LOG_ERROR("other error %d", ret);
			break;
		}
		av_frame_free(&frame);

	} while (ret == AVERROR(EAGAIN));

	if (frame)
		av_frame_free(&frame);

	if (pkt)
		av_packet_free(&pkt);

	return nullptr;
}

void AvDecoder::ready()
{
	try {
		const AVCodec* pCodec = nullptr;
		AVCodecParameters* pCodecPar = nullptr;

		if (!_formatCtx)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid _formatCtx");

		pCodecPar = _formatCtx->streams[_streamId]->codecpar;
		if (!pCodecPar)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid pCodecPar");

		pCodec = avcodec_find_decoder(pCodecPar->codec_id);
		if (!pCodec)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid pCodec");

		_codecCtx = avcodec_alloc_context3(pCodec);
		if (!_codecCtx)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "failed to alloc _codecCtx");

		if (avcodec_parameters_to_context(_codecCtx, pCodecPar) < 0)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "failed to avcodec_parameters_to_context()");

		if (pCodec->capabilities & AV_CODEC_CAP_TRUNCATED)
			_codecCtx->flags |= AV_CODEC_FLAG_TRUNCATED;

		_codecCtx->workaround_bugs = FF_BUG_AUTODETECT;

		_codecCtx->thread_type = 0;
		_codecCtx->thread_count = 0;

		if (avcodec_open2(_codecCtx, pCodec, nullptr) < 0)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "failed to avcodec_open2()");

	} catch (const Exception& e) {
		if (_codecCtx)
			avcodec_free_context(&_codecCtx);
		LOG_ERROR("%s", e.what());
		throw;
	}
}

AVPacket* AvDecoder::readFrame()
{
	AVPacket* pkt = nullptr;

	while (1) {
		pkt = av_packet_alloc();
		if (!pkt) {
			LOG_ERROR("failed to av_packet_alloc()");
			return nullptr;
		}

		int ret = av_read_frame(_formatCtx, pkt);

		if (ret < 0) {
			if (ret == AVERROR_EOF) {
				LOG_INFO("AVERROR_EOF");
				_eof = true;
			} else {
				av_packet_free(&pkt);
				__printFFmpegErrorStr("av_read_frame()", ret);
				throw Exception(OVI_ERROR_INVALID_OPERATION, "failed to av_read_frame()");
			}
		}

		if (avcodec_get_type(_formatCtx->streams[pkt->stream_index]->codecpar->codec_id) != _mediaType) {
			av_packet_free(&pkt);
			if (_eof)
				return nullptr;
			continue;
		}

		break;
	}

	//ToDo. The following can be removed if ffmpeg is 5.0 or higher, because AVFrame have time_base.
	_time_base = _formatCtx->streams[pkt->stream_index]->time_base;

	return pkt;
}

FramePackPtr AvDecoder::frame(double framerate, int64_t duration)
{
	AVFrame* frame = decode();

	if (!frame)
		return nullptr;

	FramePackPtr oviFrame = _packer->pack(frame, _frameNum, av_q2d(_time_base) * frame->pts, framerate, duration);
	av_frame_free(&frame);

	return oviFrame;
}

size_t AvDecoder::frameNum() const
{
	return _frameNum;
}

std::unique_ptr<AvDecoder> AvDecoderFactory::createAudioDecoder(int streamId, const std::string& mediaPath)
{
	return std::make_unique<AvDecoder>(AVMEDIA_TYPE_AUDIO, streamId, mediaPath,
									FramePackerFactory::create(MEDIA_TYPE_AUDIO));
}

std::unique_ptr<AvDecoder> AvDecoderFactory::createVideoDecoder(int streamId, const std::string& mediaPath)
{
	return std::make_unique<AvDecoder>(AVMEDIA_TYPE_VIDEO, streamId, mediaPath,
									FramePackerFactory::create(MEDIA_TYPE_VIDEO));
}

FrameExtractorFFMPEG::FrameExtractorFFMPEG(const std::string& mediaPath)
{
	//av_log_set_level(AV_LOG_TRACE);
	av_log_set_level(AV_LOG_ERROR);

	if (mediaPath.empty())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "empty path");

	LOG_DEBUG("media_path:%s", mediaPath.c_str());

	if (access(mediaPath.c_str(), R_OK) < 0) {
		if (errno == EACCES || errno == EPERM)
			throw Exception(OVI_ERROR_PERMISSION_DENIED, "Fail to open path: Permission Denied");

		throw Exception(OVI_ERROR_NO_SUCH_FILE, "Fail to open path: Invalid Path");
	}

	setup(mediaPath);
}

FramePackPtr FrameExtractorFFMPEG::nextVideo() const
{
	if (!_videoDecoder)
		throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid _videoDecoder");

	return _videoDecoder->frame(_videoInfo->framerate(), _videoInfo->frameCnt());
}

FramePackPtr FrameExtractorFFMPEG::nextAudio() const
{
	if (!_audioDecoder)
		throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid _audioDecoder");

	return _audioDecoder->frame(_audioInfo->framerate(), _audioInfo->frameCnt());
}

void FrameExtractorFFMPEG::setup(const std::string& mediaPath)
{
	AVFormatContext* pFormatCtx = nullptr;

	LOG_INFO("media_path: %s", mediaPath.c_str());

	try {
		pFormatCtx = __openFFmpeg(mediaPath);
		if (!pFormatCtx)
			throw Exception(OVI_ERROR_INVALID_OPERATION, "failed to _formatCtx");

		_videoInfo = std::unique_ptr<MediaInfo>(new VideoInfo(pFormatCtx, mediaPath));
		if (_videoInfo->hasStream()) {
			LOG_INFO("has video");
			_videoDecoder = AvDecoderFactory::createVideoDecoder(_videoInfo->streamId(), mediaPath);
		}

		_audioInfo = std::unique_ptr<MediaInfo>(new AudioInfo(pFormatCtx, mediaPath));
		if (_audioInfo->hasStream()) {
			LOG_INFO("has audio");
			_audioDecoder = AvDecoderFactory::createAudioDecoder(_audioInfo->streamId(), mediaPath);
		}

		if (!_videoDecoder && !_audioDecoder)
			throw Exception(OVI_ERROR_NOT_SUPPORTED_MEDIA, "no av stream");

	} catch(const Exception& e) {
		if (pFormatCtx)
			avformat_close_input(&pFormatCtx);
		LOG_ERROR("%s", e.what());
		throw;
	}

	avformat_close_input(&pFormatCtx);
}

double FrameExtractorFFMPEG::videoFramerate() const
{
	return _videoInfo->framerate();
}

double FrameExtractorFFMPEG::audioFramerate() const
{
	return _audioInfo->framerate();
}

int64_t FrameExtractorFFMPEG::videoFrames() const
{
	return _videoInfo->frameCnt();
}

int64_t FrameExtractorFFMPEG::audioFrames() const
{
	return _audioInfo->frameCnt();
}

bool FrameExtractorFFMPEG::hasVideo() const
{
	return static_cast<bool>(_videoDecoder);
}

bool FrameExtractorFFMPEG::hasAudio() const
{
	return static_cast<bool>(_audioDecoder);
}

int MediaInfo::streamId() const
{
	return _streamId;
}

bool MediaInfo::hasStream() const
{
	return (_streamId != -1);
}

double MediaInfo::framerate() const
{
	return _framerate;
}

int64_t MediaInfo::frameCnt() const
{
	return _frames;
}

VideoInfo::VideoInfo(AVFormatContext* formatCtx, const std::string& mediaPath)
{
	auto vInfo = __getVideoInfo(formatCtx, mediaPath);

	if (vInfo.has_value()) {
		_streamId = vInfo->stream_id;
		_framerate = vInfo->fps;
		_frames = vInfo->frames;
	}
}

AudioInfo::AudioInfo(AVFormatContext* formatCtx, const std::string& mediaPath)
{
	auto aInfo = __getAudioInfo(formatCtx, mediaPath);

	if (aInfo.has_value()) {
		_streamId = aInfo->stream_id;
		_framerate = aInfo->fps;
		_frames = aInfo->frames;
	}
}
