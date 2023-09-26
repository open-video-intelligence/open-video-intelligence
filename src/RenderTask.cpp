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

#include "RenderTask.h"
#include "Log.h"

using namespace ovi;

RenderTask::RenderTask(const std::string& mediaPath,
					std::shared_ptr<PluginManager> pluginManager,
					const std::string& renderId,
					MediaType type,
					int64_t videoFrames,
					double framerate,
					const std::vector<RawData>& accumulated,
					std::shared_ptr<IInvokable> completeCb,
					const std::string& outputPath)
	: _pluginManager(pluginManager)
{
	_future = std::async([=] {
		LOG_DEBUG("Entering task...");

		auto renderObj = dynamic_cast<IPluginRender*>(pluginManager->find(renderId).plugin);
		assert(renderObj);

		try {
			TimelineHelper th = TimelineHelper(framerate);
			//TODO: Need to fix the below after multi-tracks supporting.
			th.appendTrack("Track-001", type);
			th.makeMediaRef(mediaPath, framerate, videoFrames);

			auto trs = makeTimeRange(accumulated, framerate);
			for (const auto& tr : trs) {
				th.appendClip(
							"Track-001",
							std::string(),
							tr.timeRange,
							mediaPath,
							makeEffectList(tr.collection));
			}

			renderObj->setAttrs({ {"path", outputPath} });
			renderObj->render(th.getTimeline());

			completeCb->invoke(OVI_ERROR_NONE);
		} catch (const Exception& e) {
			LOG_ERROR("%s", e.what());
			completeCb->invoke(static_cast<ovi_error_e>(e.error()));
		}

		LOG_DEBUG("task terminated");
	});
}

RenderTask::~RenderTask()
{
	LOG_ENTER();

	_future.get();

	LOG_INFO("Task get done!");
}

std::vector<TimeRangeWithMetadata> RenderTask::makeTimeRange(const std::vector<RawData>& accumulated, double framerate) const
{
	DataAnalyzer da = DataAnalyzer();

	da.setCorrectionValue(framerate);
	da.analyzeRawData(accumulated);

	return da.result();
}

std::vector<effectRetainer> RenderTask::makeEffectList(SortedCollection collection)
{
	std::vector<effectRetainer> result;

	for (const auto& [ uid, details ] : collection) {
		auto effect = initializeEffect(_pluginManager->find(uid));
		auto& dic = effect->metadata();

		for (const auto& detected : details)
			dic[std::to_string(detected.frameNumber)] = fillFrameEffect(detected.list);

		result.push_back(effect);
	}

	return result;
}

effectRetainer RenderTask::initializeEffect(const Plugin& plugin)
{
	auto obj = dynamic_cast<IPluginEffect*>(plugin.plugin);
	assert(obj);
	auto infoList = obj->effectInfo();

	otio::AnyDictionary dic;
	std::copy(infoList.begin(), infoList.end(), std::inserter(dic, dic.end()));
	dic.erase("name");

	return new otio::Effect(std::string(), infoList["name"], dic);
}

class VisitorAddItem {
public:
	explicit VisitorAddItem(otio::AnyVector* metaDic)
		: _metaDic(metaDic) {}

	void operator()(const OVIRect& rect) {
		_metaDic->push_back(
			otio::AnyDictionary {
				{ "x", rect.x },
				{ "y", rect.y },
				{ "width", rect.width },
				{ "height", rect.height }
			}
		);
	}

	void operator()(const OVIRectTag& rectTag) {
		_metaDic->push_back(
			otio::AnyDictionary {
				{ "x", rectTag.x },
				{ "y", rectTag.y },
				{ "width", rectTag.width },
				{ "height", rectTag.height },
				{ "tag", rectTag.tag }
			}
		);
	}

	void operator()(const double& value) {
		_metaDic->push_back(
			otio::AnyDictionary {
				{ "value", value },
			}
		);
	}

private:
	otio::AnyVector* _metaDic;
};

otio::AnyVector RenderTask::fillFrameEffect(Details list)
{
	otio::AnyVector metaDic;

	if (list.empty())
		return metaDic;

	for (const auto& item : list) {
		std::visit(VisitorAddItem { &metaDic }, item);
	}

	return metaDic;
}
