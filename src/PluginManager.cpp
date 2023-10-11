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

#include "PluginManager.h"
#include "IPluginRender.h"
#include "Log.h"

using namespace ovi;

PluginManager::~PluginManager()
{
	unloadAll();
}

std::string PluginManager::makeId(const std::string& name)
{
	static int s_num = 0;
	return (name + "." + std::to_string(++s_num));
}

const std::string& PluginManager::load(const std::string& name)
{
	std::string uid = makeId(name);
	_loadedPlugins.insert({ uid, PluginLoader::instance().load(name) });

	LOG_DEBUG("plugin name:%s", name.c_str());

	auto plugin = _loadedPlugins.find(uid);

	return plugin->first;
}

const Plugin& PluginManager::find(const std::string& uid) const
{
	auto iter = _loadedPlugins.find(uid);

	if (iter == _loadedPlugins.end())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, std::string { "No plugin: " } + uid);

	return iter->second;
}

bool PluginManager::exist(const std::string& uid) const
{
	auto iter = _loadedPlugins.find(uid);

	return (iter != _loadedPlugins.end());
}

void PluginManager::unloadAll()
{
	for (auto iter = _loadedPlugins.begin(); iter != _loadedPlugins.end(); iter++)
		PluginLoader::instance().unload(iter->second);

	_loadedPlugins.clear();
}

void PluginManager::setAttrs(const std::string& uid, const std::map<std::string, std::string>& attrs)
{
	auto iter = _loadedPlugins.find(uid);

	if (iter == _loadedPlugins.end())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, std::string { "No plugin: " } + uid);

	for (const auto& attr : attrs)
		iter->second.attrs.insert_or_assign(attr.first, attr.second);
}

void PluginManager::validate(bool hasVideo, bool hasAudio) const
{
	for (const auto& [uid, plugin] : _loadedPlugins) {	//ToDo. check only the plugins linked
		if (!hasVideo && (plugin.type == PLUGIN_TYPE_VIDEO_DETECT || plugin.type == PLUGIN_TYPE_VIDEO_EFFECT))
			throw Exception(OVI_ERROR_INVALID_OPERATION, "No video but request video detect or effect");

		if (!hasAudio && (plugin.type == PLUGIN_TYPE_AUDIO_DETECT || plugin.type == PLUGIN_TYPE_AUDIO_EFFECT))
			throw Exception(OVI_ERROR_INVALID_OPERATION, "No audio but request audio detect or effect");
	}
}

void PluginManager::validateAttrs(const std::string& renderUid) const
{
	auto renderPlugin = find(renderUid);
	auto renderObj = dynamic_cast<IPluginRender*>(renderPlugin.plugin);
	assert(renderObj);

	for (const auto& [uid, plugin] : _loadedPlugins) {	//ToDo. check only the plugins linked
		if (plugin.type == PLUGIN_TYPE_VIDEO_EFFECT || plugin.type == PLUGIN_TYPE_AUDIO_EFFECT) {
			if (plugin.attrs.empty())
				throw Exception(OVI_ERROR_INVALID_OPERATION, std::string { "No effect info: " } + uid);

			renderObj->validateEffectAttrs(plugin.attrs);
		} else {
			//ToDo
		}
	}
}

void PluginManager::setAllAttrs()
{
	for (const auto& plugin : _loadedPlugins) {
	//ToDo :: validate check here..
		if (!plugin.second.attrs.empty()) {

				plugin.second.plugin->setAttrs(plugin.second.attrs);
		} else {
			//TODO: No attribute cases..
		}
	}
}

const std::string& PluginManager::getAttr(const std::string& uid, const std::string& key) const
{
	auto iter = _loadedPlugins.find(uid);

	if (iter == _loadedPlugins.end())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, std::string { "No plugin: " } + uid);

	return iter->second.attrs.at(key);
}

MetaForm PluginManager::getMetaForm(const std::string& uid, const std::string& effectName) const
{
	auto iter = _loadedPlugins.find(uid);

	switch(iter->second.type) {
	case PLUGIN_TYPE_VIDEO_DETECT:
	case PLUGIN_TYPE_VIDEO_EFFECT:
	case PLUGIN_TYPE_AUDIO_DETECT:
	case PLUGIN_TYPE_AUDIO_EFFECT:
		return iter->second.metaForm;

	case PLUGIN_TYPE_RENDER:
	{
		auto renderObj = dynamic_cast<IPluginRender*>(iter->second.plugin);
		return renderObj->effectMetaForm(effectName);
	}

	default:
		return METAFORM_NONE;
	}
}
