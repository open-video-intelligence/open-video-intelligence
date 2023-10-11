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

#include "Session.h"
#include "PerformanceMeasure.h"
#include "Log.h"
#include "ovi_types.h"

#include <string>
#include <filesystem>

using namespace ovi;

static std::string stateInfo[] = {
	[OVI_STATE_IDLE] = "OVI_STATE_IDLE",
	[OVI_STATE_ANALYSIS] = "OVI_STATE_ANALYSIS",
	[OVI_STATE_RENDER] = "OVI_STATE_RENDER",
};

void Session::completeCb(void* handle, ovi_error_e error, void* userData)
{
	auto session = static_cast<Session*>(handle);

	LOG_INFO("completeCb called. state:%s, error:%d", stateInfo[session->_state].c_str(), error);

	if (error != OVI_ERROR_NONE) {
		if (session->_errorCb)
			session->_errorCb->invoke(error);

		session->updateState(OVI_STATE_IDLE);
		return;
	}

	if (session->_state == OVI_STATE_ANALYSIS) {
		PerformanceMeasure::instance().split("analysis");
		session->runRender();
		session->updateState(OVI_STATE_RENDER);

	} else if (session->_state == OVI_STATE_RENDER) {
		PerformanceMeasure::instance().split("render");
		PerformanceMeasure::instance().stop();
		session->updateState(OVI_STATE_IDLE);
	} else {
		//Stopped
	}
}

void Session::updateState(ovi_state_e current)
{
	ovi_state_e previous = _state;
	_state = current;

	if (_stateChangedCb && (previous != current)) {
		LOG_DEBUG("invoke state changed callback. previous:%d, current:%d", previous, current);
		_stateChangedCb->invoke((ovi_state_e)previous, (ovi_state_e)current);
	}
}

Session::Session()
	: _pluginManager(std::make_shared<PluginManager>()),
	_completeCb(std::shared_ptr<IInvokable>(new ErrorCallback(this, completeCb, nullptr)))
{
	updateState(OVI_STATE_IDLE);
}

Session::~Session()
{
	try {
		destroy();
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		// dtor should not throw exception!!!!
	}
}

void Session::setMediaPath(const std::string& mediaPath)
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state");

	if (mediaPath.empty())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid mediaPath");

	std::error_code err;
	auto pathObj = std::filesystem::canonical(mediaPath, err);
	if (err.value() == EINVAL)
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "Invalid mediaPath");

	if (err.value() == ENOENT)
		throw Exception(OVI_ERROR_NO_SUCH_FILE, "No such file");

	_mediaPath = pathObj.string();
}

void Session::setErrorCb(ovi_error_cb callback, void* userData)
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state");

	if (!callback)
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid callback");

	_errorCb = std::unique_ptr<IInvokable>(new ErrorCallback(this, callback, userData));
}

void Session::unsetErrorCb()
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state");

	if (!_errorCb)
		throw Exception(OVI_ERROR_INVALID_OPERATION, "callback was not set");

	_errorCb.reset();
}

void Session::setProgressCb(ovi_progress_cb callback, void* userData)
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state");

	if (!callback)
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid callback");

	_progress_cb.callback = reinterpret_cast<void*>(callback);
	_progress_cb.userData = userData;
}

void Session::unsetProgressCb()
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state");

	if (!_progress_cb.callback)
		throw Exception(OVI_ERROR_INVALID_OPERATION, "callback was not set");

	_progress_cb.callback = nullptr;
	_progress_cb.userData = nullptr;
}

void Session::setStateChangedCb(ovi_state_changed_cb callback, void* userData)
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state");

	if (!callback)
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid callback");

	_stateChangedCb = std::unique_ptr<IInvokable>(new StateChangedCallback(this, callback, userData));
}

void Session::unsetStateChangedCb()
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state");

	if (!_stateChangedCb)
		throw Exception(OVI_ERROR_INVALID_OPERATION, "callback was not set");

	_stateChangedCb.reset();
}

void Session::start()
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state");

	if (!_logicAnalyzer)
		throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid _logicAnalyzer");

	if (!_pluginManager)
		throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid _pluginManager");

	if (_mediaPath.empty())
		throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid _mediaPath");

	if (_renderUid.empty())
		throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid _renderUid");

	if (_outputFilePath.empty())
		throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid _outputFilePath");

	if (!validate_link(_logicAnalyzer.get(), _pluginManager.get(), _renderUid))
		throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid link");

	_frameExtractor = std::shared_ptr<IFrameExtractor>(FrameExtractorFactory::create(_mediaPath));

	_pluginManager->validate(_frameExtractor->hasVideo(), _frameExtractor->hasAudio());
	_pluginManager->validateAttrs(_renderUid);
	_pluginManager->setAllAttrs();

	_accumulator = std::make_shared<Accumulator>();
	_avSynchronizer = std::make_shared<AvSynchronizer>(_frameExtractor);

	PerformanceMeasure::instance().start();
	runDataFlow();

	updateState(OVI_STATE_ANALYSIS);
}

void Session::stop()
{
	if (_state != OVI_STATE_ANALYSIS)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state :" + stateInfo[_state]);

	if (!_dataFlow)
		throw Exception(OVI_ERROR_INVALID_OPERATION, "invalid _dataFlow");

	updateState(OVI_STATE_IDLE);

	_dataFlow->stop();

	PerformanceMeasure::instance().stop();
}

void Session::destroy()
{
	if (!_dataFlow)
		return;

	if (_state == OVI_STATE_IDLE)
		_dataFlow->stop();
	else
		stop();
}

void Session::setRender(const std::string& uid, std::string outputPath)
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state :" + stateInfo[_state]);

	if (uid.empty())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid uid");

	if (outputPath.empty())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid outputPath");

	Plugin plugin = _pluginManager->find(uid);

	if (plugin.type != PLUGIN_TYPE_RENDER)
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "not render uid");

	_renderUid = uid;
	_outputFilePath = outputPath;
}

void Session::registerPlugin(const std::vector<std::string>& request)
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state :" + stateInfo[_state]);

	if (request.empty())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "empty request");

	if (!validate_logic(request, _pluginManager.get()))
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid request");

	_logicAnalyzer = std::make_shared<LogicAnalyzer>(request, _pluginManager.get());
}

const std::string& Session::appendPlugin(const std::string& name)
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state :" + stateInfo[_state]);

	if (name.empty())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid name");

	return _pluginManager->load(name);
}

void Session::setPluginAttrs(const std::string& uid, const std::map<std::string, std::string>& attrs)
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state :" + stateInfo[_state]);

	if (uid.empty())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "invalid uid");

	_pluginManager->setAttrs(uid, attrs);
}

ovi_state_e Session::state()
{
	return _state;
}

void Session::runDataFlow()
{
	_dataFlow = std::make_unique<DataFlow>(
										_avSynchronizer,
										_logicAnalyzer,
										_pluginManager,
										_accumulator,
										_completeCb,
										((_frameExtractor->hasVideo()) ? _skipFrames : 0));

	if (_progress_cb.callback)
		_dataFlow->setProgressCallback(this,
									(ovi_progress_cb)_progress_cb.callback,
									_progress_cb.userData);

	_dataFlow->start();
}

void Session::runRender()
{
	//ToDo: RenderTask refactoring
	const auto [ type, frames, framerate ] = [&] {
		if (_frameExtractor->hasAudio() && !_frameExtractor->hasVideo())
			return std::make_tuple(MEDIA_TYPE_AUDIO,
								_frameExtractor->audioFrames(),
								_frameExtractor->audioFramerate());
		else
			return std::make_tuple(MEDIA_TYPE_VIDEO,
								_frameExtractor->videoFrames(),
								_frameExtractor->videoFramerate());
	}();

	_render = std::make_unique<RenderTask>(_mediaPath,
										_pluginManager,
										_renderUid,
										type,
										frames,
										framerate,
										_accumulator->accumulated(),
										_completeCb,
										_outputFilePath);
}

void Session::setSkipVideoFrames(size_t frames)
{
	if (_state != OVI_STATE_IDLE)
		throw Exception(OVI_ERROR_INVALID_STATE, "invalid _state :" + stateInfo[_state]);

	_skipFrames = frames;
}
