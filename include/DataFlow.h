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

#ifndef __OPEN_VIDEO_INTELLIGENCE_DATA_FLOW_H__
#define __OPEN_VIDEO_INTELLIGENCE_DATA_FLOW_H__

#include "IFrameExtractor.h"
#include "LogicAnalyzer.h"
#include "Accumulator.h"
#include "AvSynchronizer.h"
#include "Callback.h"
#include "FramePack.h"
#include "OutcomeCache.h"
#include "ThreadRunner.h"

#include <string>
#include <thread>
#include <atomic>

namespace ovi {

class DataFlow : public ThreadRunner
{
public:
	explicit DataFlow(std::shared_ptr<AvSynchronizer> avSynchronizer,
			std::shared_ptr<LogicAnalyzer> logicAnalyzer,
			std::shared_ptr<PluginManager> pluginManager,
			std::shared_ptr<Accumulator> accumulator,
			std::shared_ptr<IInvokable> completeCb,
			size_t skipFrames);
	~DataFlow();

	void setProgressCallback(void* handle, ovi_progress_cb callback, void* userData);

private:
	void worker() override;
	void appendResult(const FramePack* vFrame, std::vector<FramePackPtr>& aFrames, const DetectedData& detected);
	void updateAllResult(const Details& detected);
	Outcome processPlugin(const std::string& uid, FramePack* vFrame, std::vector<FramePackPtr>& aFrames);
	void invokeProgressCb(std::string progress);

	std::shared_ptr<AvSynchronizer> _avSynchronizer;
	std::shared_ptr<LogicAnalyzer> _logicAnalyzer;
	std::shared_ptr<PluginManager> _pluginManager;
	std::shared_ptr<Accumulator> _accumulator;

	std::shared_ptr<IInvokable> _completeCb;
	std::unique_ptr<IInvokable> _progressCallback;

	OutcomeCache _outcomeCache;

	size_t _skipFrames;
};

}

#endif // __OPEN_VIDEO_INTELLIGENCE_DATA_FLOW_H__
