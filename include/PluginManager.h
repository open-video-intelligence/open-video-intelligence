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

#ifndef __OPEN_VIDEO_INTELLIGENCE_PLUGIN_MANAGER_H__
#define __OPEN_VIDEO_INTELLIGENCE_PLUGIN_MANAGER_H__

#include "PluginLoader.h"

namespace ovi {

class PluginManager
{
public:
	PluginManager() = default;
	~PluginManager();

	const std::string& load(const std::string& name);
	const Plugin& find(const std::string& uid) const;
	bool exist(const std::string& uid) const;
	void validate(bool hasVideo, bool hasAudio) const;

	void setAllAttrs();
	void setAttrs(const std::string& uid, const std::map<std::string, std::string>& attrs);
	const std::string& getAttr(const std::string& uid, const std::string& key) const;
	void validateAttrs(const std::string& renderUid) const;

	MetaForm getMetaForm(const std::string& uid, const std::string& effectName) const;

private:
	void unloadAll();

	std::string makeId(const std::string& name);
	std::map<std::string, Plugin> _loadedPlugins;
};

} // ovi

#endif // __OPEN_VIDEO_INTELLIGENCE_PLUGIN_MANAGER_H__
