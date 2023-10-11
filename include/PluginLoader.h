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

#ifndef __OPEN_VIDEO_INTELLIGENCE_PLUGIN_LOADER_H__
#define __OPEN_VIDEO_INTELLIGENCE_PLUGIN_LOADER_H__

#include <memory>
#include "IPluginProcess.h"
#include "IPluginEffect.h"
#include "IPlugin.h"

namespace ovi {

struct Plugin {
	PluginType type {};
	std::vector<int> formats {};
	MetaForm metaForm {};
	IPlugin* plugin {};
	void* dlHandle {};
	std::map<std::string, std::string> attrs;
};

typedef enum {
	LANG_C,
	LANG_PYTHON,
} language_e;

struct PluginInfo {
	language_e lang {};
	std::string name;
	PluginType type {};
	std::vector<int> formats {};
	MetaForm metaForm {};
	std::string description;
	std::string libraryPath;
	std::vector<Attribute> attrs;
};

class PyManager;

class PluginLoader
{
public:
	static PluginLoader& instance() {
		static PluginLoader _instance;
		return _instance;
	}

	const std::vector<Attribute>& getPluginAttrs(const std::string& name) const;
	const std::vector<PluginInfo>& getAvailablePluginList() const;

	Plugin load(const std::string& name);
	void unload(Plugin pluginObj);

private:
	PluginLoader();
	~PluginLoader();
	PluginLoader(PluginLoader& other) = delete;
	void operator=(const PluginLoader&) = delete;

	Plugin create(PluginInfo& info);
	void getSharedPathList(const std::string& pluginDir);
#ifdef OVI_ENABLE_PYTHON
	void addPluginInfoForPy(const std::string& pluginPath, const std::string& moduleName);
#endif /* OVI_ENABLE_PYTHON */
	void addPluginInfo(const std::string& pluginPath);

	std::vector<std::string> _sharedPathList;
	std::vector<PluginInfo> _availablePlugins;
#ifdef OVI_ENABLE_PYTHON
	bool _pyintf {};
	std::shared_ptr<PyManager> _pyManager;
#endif /* OVI_ENABLE_PYTHON */
};

}

#endif // __OPEN_VIDEO_INTELLIGENCE_PLUGIN_LOADER_H__
