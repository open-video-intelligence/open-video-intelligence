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

#ifdef OVI_ENABLE_PYTHON

#include "PyPluginProcess.h"
#include "PyManager.h"

using namespace ovi;

PyPlugin::PyPlugin(std::shared_ptr<PyManager> pyManager, const std::string& moduleName)
{
	_pyManager = pyManager;
	_pluginId = _pyManager->create(moduleName);
}

PyPlugin::~PyPlugin()
{
	_pyManager->remove(_pluginId);
}

int PyPlugin::setAttrs(const std::map<std::string, std::string>& attrs)
{
	return _pyManager->setAttributes(_pluginId, attrs);
}

Outcome PyPlugin::process(ovi::FramePack* frame)
{
	return _pyManager->process(_pluginId, frame);
}

#endif /* OVI_ENABLE_PYTHON */