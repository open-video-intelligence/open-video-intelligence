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

#ifndef __OPEN_VIDEO_INTELLIGENCE_RENDER_TASK_H__
#define __OPEN_VIDEO_INTELLIGENCE_RENDER_TASK_H__

#include "FrameExtractorFactory.h"
#include "PluginManager.h"
#include "Callback.h"
#include "DataAnalyzer.h"

#include <string>
#include <future>

namespace ovi {

class RenderTask
{
public:
	RenderTask(const std::string& mediaPath,
			std::shared_ptr<PluginManager> pluginManager,
			const std::string& renderId,
			MediaType type,
			int64_t videoFrames,
			double framerate,
			const std::vector<RawData>& accumulated,
			std::shared_ptr<IInvokable> completeCb,
			const std::string& outputPath);
	~RenderTask();

private:
	std::vector<TimeRangeWithMetadata> makeTimeRange(const std::vector<RawData>& accumulated, double framerate) const;
	std::vector<effectRetainer> makeEffectList(SortedCollection collection);
	effectRetainer initializeEffect(const Plugin& plugin);
	otio::AnyVector fillFrameEffect(Details list);

	std::future<void> _future;
	std::shared_ptr<PluginManager> _pluginManager;
};

}

#endif // __OPEN_VIDEO_INTELLIGENCE_RENDER_TASK_H__
