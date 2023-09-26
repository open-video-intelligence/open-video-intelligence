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

#include "DataFlow.h"
#include "Log.h"

using namespace ovi;

DataFlow::DataFlow(std::shared_ptr<AvSynchronizer> avSynchronizer,
				std::shared_ptr<LogicAnalyzer> logicAnalyzer,
				std::shared_ptr<PluginManager> pluginManager,
				std::shared_ptr<Accumulator> accumulator,
				std::shared_ptr<IInvokable> completeCb,
				size_t skipFrames)
	: ThreadRunner(), _avSynchronizer(avSynchronizer), _logicAnalyzer(logicAnalyzer),
	_pluginManager(pluginManager), _accumulator(accumulator),
	_completeCb(completeCb),
	_skipFrames(skipFrames)
{
}

DataFlow::~DataFlow()
{
	LOG_ENTER();
}

void DataFlow::worker()
{
	LOG_DEBUG("Entering thread..");
	int ret = OVI_ERROR_NONE;

	invokeProgressCb("Start Analysis...");

	while (_run.load()) {
		FramePackPtr vFrame;
		std::vector<FramePackPtr> aFrames;

		try {
			for (size_t i = 0; i < _skipFrames + 1; i++) {
				vFrame = _avSynchronizer->getNextVideo();
				aFrames = _avSynchronizer->getNextAudio();

				if (!vFrame && aFrames.empty())
					break;
			}

			if (!vFrame && aFrames.empty())
				break;

		} catch (const Exception& e) {
			LOG_ERROR("%s", e.what());
			ret = e.error();
			break;
		}

		_logicAnalyzer->reset();
		_outcomeCache.clear();
		while (_run.load()) {
			std::string uid = _logicAnalyzer->nextPlugin(_outcomeCache.result().detect);
			LOG_DEBUG("plugin:%s", uid.c_str());

			if (uid == OVI_EOP) {
				// multi-frame detector check..
				if (_outcomeCache.findMultiFrameResult())
					updateAllResult(_outcomeCache.getMultiFrameResult());
				else
					appendResult(vFrame.get(), aFrames, _outcomeCache.detected());
				break;
			}

			if (_outcomeCache.hit(uid)) {
				_outcomeCache.setResultUid(uid);
				continue;
			}

			const auto& plugin = _pluginManager->find(uid);
			if (plugin.type == PLUGIN_TYPE_VIDEO_EFFECT || plugin.type == PLUGIN_TYPE_AUDIO_EFFECT) {
				_outcomeCache.setDetected(uid);
				continue;
			}

			try {
				_outcomeCache.write(uid, processPlugin(uid, vFrame.get(), aFrames));
			} catch (const Exception& e) {
				LOG_ERROR("%s", e.what());
				ret = e.error();
				break;
			}
		}

		std::string progressStr;
		if (vFrame) {
			progressStr += std::to_string(vFrame->frameNum());
			progressStr += "/";
			progressStr += std::to_string(vFrame->duration());
		} else if (!aFrames.empty()) {
			progressStr += std::to_string(aFrames[0]->frameNum());
			progressStr += "/";
			progressStr += std::to_string(aFrames[0]->duration());
		}
		if (!progressStr.empty())
			invokeProgressCb(progressStr);
	}

	invokeProgressCb("Finish Analysis...");

	_run.store(false);

	_completeCb->invoke((ovi_error_e)ret);

	LOG_DEBUG("thread is terminated");
}

void DataFlow::setProgressCallback(void* handle, ovi_progress_cb callback, void* userData)
{
	_progressCallback = std::unique_ptr<IInvokable>(new ProgressCallback(handle, callback, userData));
}

void DataFlow::appendResult(const FramePack* vFrame, std::vector<FramePackPtr>& aFrames, const DetectedData& detected)
{
	if (vFrame) {
		for (int i = _skipFrames; i >= 0; i--)
			_accumulator->append(vFrame->frameNum() - i, _logicAnalyzer->include(), detected);

	} else {
		for (size_t i = 0; i < aFrames.size(); i++)
			_accumulator->append(aFrames[i]->frameNum(), _logicAnalyzer->include(), detected);
	}
}

void DataFlow::updateAllResult(const Details& detected)
{
	_accumulator->update(detected);
}

Outcome DataFlow::processPlugin(const std::string& uid, FramePack* vFrame, std::vector<FramePackPtr>& aFrames)
{
	Outcome result = { true, {} };

	auto& plugin = _pluginManager->find(uid);
	auto processObj = dynamic_cast<IPluginProcess*>(plugin.plugin);
	assert(processObj);

	switch (plugin.type) {
	case PLUGIN_TYPE_VIDEO_DETECT:
		if (vFrame) {
			// FixMe: always copy here?
			FramePackPtr frameConverted =
				vFrame->convert(plugin.formats);
			result = processObj->process(frameConverted.get());
		}
		break;

	case PLUGIN_TYPE_AUDIO_DETECT:
		for (size_t i = 0; i < aFrames.size(); i++) {
			// FixMe: always copy here?
			FramePackPtr frameConverted =
				aFrames[i].get()->convert(plugin.formats);
			result = processObj->process(frameConverted.get());

			if (result.detect == true)	//ToDo. If at least one is true, it is considered to be true as a whole.
				break;
		}
		break;

	default:
		LOG_ERROR("Not supported plugin type:%d", plugin.type);
		break;
	}

	return result;
}

void DataFlow::invokeProgressCb(std::string progress)
{
	if (_progressCallback)
		_progressCallback->invoke({progress});
}
