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

#include <dlfcn.h>
#include <stdio.h>
#include <filesystem>
#include <algorithm>

#include "PluginLoader.h"
#include "Log.h"
#ifdef OVI_ENABLE_PYTHON
#include "PyPluginProcess.h"
#include "PyManager.h"
#endif /* OVI_ENABLE_PYTHON */

using namespace ovi;

/* For dlopen */
typedef class IPlugin *(*createPlugin)();
typedef void (*destroyPlugin)(class IPlugin *);
typedef const char *(*name)();
typedef PluginType (*type)();
typedef std::vector<int>* (*supportFormat)();
typedef MetaForm (*supportMetaForm)();
typedef const char *(*description)();
typedef void *(*attributeList)();

void PluginLoader::getSharedPathList(const std::string& pluginDir)
{
	std::error_code ec {};
	std::filesystem::directory_iterator dit(pluginDir, ec);

	if (!ec) {
		for (const auto& entry : dit) {
			if (!entry.is_regular_file())
				continue;

			if (entry.path().extension().compare(".so") == 0)
				addPluginInfo(entry.path());

#ifdef OVI_ENABLE_PYTHON
			if (!_pyintf) {
				if (entry.path().extension().compare(".py") == 0 &&
					entry.path().stem().compare("__init__") != 0)
					addPluginInfoForPy(entry.path(), entry.path().stem());
			}
#endif /* OVI_ENABLE_PYTHON */
		}
	} else {
		LOG_ERROR("directory_iterator failed: %d", ec.value());
	}
}

void PluginLoader::addPluginInfo(const std::string& pluginPath)
{
	void* handle = nullptr;
	LOG_ENTER();

	try {
		handle = dlopen(pluginPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!handle)
			throw Exception(OVI_ERROR_INVALID_PARAMETER, std::string { "dlopen failed: " } + dlerror());

		auto nameFunc = reinterpret_cast<name>(dlsym(handle, "name"));
		if (!nameFunc)
			throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "name dlsym failed: " } + dlerror());

		auto typeFunc = reinterpret_cast<type>(dlsym(handle, "type"));
		if (!typeFunc)
			throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "type dlsym failed: " } + dlerror());

		auto formatFunc = reinterpret_cast<supportFormat>(dlsym(handle, "supportFormat"));
		if (!formatFunc)
			throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "supportFormat dlsym failed: " } + dlerror());

		auto metaFormFunc = reinterpret_cast<supportMetaForm>(dlsym(handle, "supportMetaForm"));
		if (!metaFormFunc)
			throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "supportMetaForm dlsym failed: " } + dlerror());

		auto descFunc = reinterpret_cast<description>(dlsym(handle, "description"));
		if (!descFunc)
			throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "description dlsym failed: " } + dlerror());

		auto attrFunc = reinterpret_cast<attributeList>(dlsym(handle, "attributeList"));
		if (!attrFunc)
			throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "attributeList dlsym failed: " } + dlerror());

		auto attrs = reinterpret_cast<std::vector<Attribute>*>(attrFunc());
		_availablePlugins.push_back( { LANG_C,
							nameFunc(),
							typeFunc(),
							*formatFunc(),
							metaFormFunc(),
							descFunc(),
							pluginPath,
							*attrs } );

		dlclose(handle);
	} catch (const Exception& e) {
		LOG_ERROR("ERROR: %s", e.what());
		if (handle)
			dlclose(handle);
		throw;
	}
	LOG_LEAVE();
}

#ifdef OVI_ENABLE_PYTHON
void PluginLoader::addPluginInfoForPy(const std::string& pluginPath, const std::string& moduleName)
{
	LOG_ENTER();
	try {
		auto info = _pyManager->getPluginInfo(moduleName);
		if (info.libraryPath.compare(moduleName) == 0)
			_availablePlugins.push_back(info);
	} catch (const std::bad_variant_access& e) {
		LOG_ERROR("ERROR: %s", e.what());
	}

	LOG_LEAVE();
}
#endif /* OVI_ENABLE_PYTHON */

PluginLoader::PluginLoader()
{
#ifdef OVI_ENABLE_PYTHON
	if (Py_IsInitialized())
		_pyintf = true;
	else
		_pyManager = std::make_shared<PyManager>();
#endif /* OVI_ENABLE_PYTHON */
	getSharedPathList(PLUGIN_INSTALLED_DIR);
}

PluginLoader::~PluginLoader()
{
#ifdef OVI_ENABLE_PYTHON
	// Note: must be unique owner in this momement
	_pyManager.reset();
#endif /* OVI_ENABLE_PYTHON */
}

Plugin PluginLoader::create(PluginInfo& info)
{
	if (info.lang == LANG_C) {
		void* handle = dlopen(info.libraryPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!handle)
			throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "dlopen failed: " } + dlerror());

		auto createPluginFunc = reinterpret_cast<createPlugin>(dlsym(handle, "createPlugin"));
		if (!createPluginFunc)
			throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "dlsym failed: " } + dlerror());

		return Plugin({ info.type, info.formats, info.metaForm, createPluginFunc(), handle });
	} else if (info.lang == LANG_PYTHON) {
#ifdef OVI_ENABLE_PYTHON
		IPlugin* func = new PyPlugin(_pyManager, info.libraryPath);
		return Plugin({ info.type, info.formats, info.metaForm, func, nullptr });
#endif /* OVI_ENABLE_PYTHON */
	}

	throw Exception(OVI_ERROR_INVALID_OPERATION, "Unsupported language");
}

Plugin PluginLoader::load(const std::string& name)
{
	for (auto& item : _availablePlugins) {
		if (item.name.compare(name) == 0)
			return create(item);
	}

	throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "No plugin: " } + name);
}

void PluginLoader::unload(Plugin pluginObj)
{
	if (pluginObj.dlHandle) {
		auto destroyPluginFunc = reinterpret_cast<destroyPlugin>(dlsym(pluginObj.dlHandle, "destroyPlugin"));
		if (destroyPluginFunc)
			destroyPluginFunc(pluginObj.plugin);
		dlclose(pluginObj.dlHandle);
	} else {
		delete pluginObj.plugin;
	}
}

const std::vector<Attribute>& PluginLoader::getPluginAttrs(const std::string& name) const
{
	auto iter = std::find_if(_availablePlugins.begin(), _availablePlugins.end(),
							[&](const auto& plugin) -> bool { return plugin.name == name; });
	if (iter != _availablePlugins.end())
		return iter->attrs;

	throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "No Plugin: " } + name );
}

const std::vector<PluginInfo>& PluginLoader::getAvailablePluginList() const
{
	return _availablePlugins;
}
