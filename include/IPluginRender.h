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

#ifndef __OPEN_VIDEO_INTELLIGENCE_IPLUGIN_RENDER_H__
#define __OPEN_VIDEO_INTELLIGENCE_IPLUGIN_RENDER_H__

#include "IPlugin.h"
#include "TimelineHelper.h"

class IPluginRender : public IPlugin
{
public:
	~IPluginRender() override = default;

	virtual void validateEffectAttrs(const std::map<std::string, std::string>& attrs) {};
	virtual MetaForm effectMetaForm(const std::string& effectName) {return METAFORM_NONE;};
	virtual void render(timelineRetainer otioTimeline) = 0;
};

#endif /* __OPEN_VIDEO_INTELLIGENCE_IPLUGIN_RENDER_H__ */
