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

#ifndef __OPEN_VIDEO_INTELLIGENCE_SESSION_H__
#define __OPEN_VIDEO_INTELLIGENCE_SESSION_H__

#include "DataFlow.h"
#include "RenderTask.h"

namespace ovi {

typedef struct _ovi_callbacks {
	void* callback {};
	void* userData {};
} ovi_callbacks_s;

class Session
{
public:
	Session();
	~Session();

	void start();
	void stop();
	void destroy();
	ovi_state_e state();

	void setRender(const std::string& name, std::string outputPath);

	const std::string& appendPlugin(const std::string& name);
	void setPluginAttrs(const std::string& uid, const std::map<std::string, std::string>& attrs);
	void registerPlugin(const std::vector<std::string>& request); //TODO: Need to Rename

	void setMediaPath(const std::string& mediaPath);
	void setErrorCb(ovi_error_cb callback, void* userData);
	void unsetErrorCb();
	void setProgressCb(ovi_progress_cb callback, void* userData);
	void unsetProgressCb();
	void setStateChangedCb(ovi_state_changed_cb callback, void* userData);
	void unsetStateChangedCb();
	void setSkipVideoFrames(size_t frames);

private:
	void updateState(ovi_state_e current);
	static void completeCb(void* handle, ovi_error_e error, void* userData);
	void runDataFlow();
	void runRender();

	std::unique_ptr<DataFlow> _dataFlow;
	std::unique_ptr<RenderTask> _render;
	std::shared_ptr<LogicAnalyzer> _logicAnalyzer;
	std::shared_ptr<PluginManager> _pluginManager;
	std::shared_ptr<Accumulator> _accumulator;
	std::shared_ptr<IFrameExtractor> _frameExtractor;
	std::shared_ptr<AvSynchronizer> _avSynchronizer;

	std::string _renderUid;
	std::string _mediaPath;
	std::string _otioFilePath;
	std::string _outputFilePath;
	ovi_state_e _state;
	size_t _skipFrames {};

	ovi_callbacks_s _progress_cb {};

	std::unique_ptr<IInvokable> _errorCb;
	std::unique_ptr<IInvokable>_stateChangedCb;
	std::shared_ptr<IInvokable> _completeCb;

};

}

#endif // __OPEN_VIDEO_INTELLIGENCE_SESSION_H__
