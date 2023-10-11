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

#ifndef __OPEN_VIDEO_INTELLIGENCE_IPLUGIN_PROCESS_H__
#define __OPEN_VIDEO_INTELLIGENCE_IPLUGIN_PROCESS_H__

#include <variant>

#include "IPlugin.h"
#include "FramePack.h"

// Convert ORIGIN_LEFTTOP typed box in tflite to {x, y, w, h}
#define TLBR2XYWH(T, W, H) { *(T+1)*W, *T*H, (*(T+3)-*(T+1))*W, (*(T+2)-*T)*H }
#define TLBR2XYWH_S(T, M, S) { *(T+1)*M/S, *T*M/S, (*(T+3)-*(T+1))*M/S, (*(T+2)-*T)*M/S }

struct OVIRect {
	double x {};
	double y {};
	double width {};
	double height {};
};

struct OVIRectTag {
	double x {};
	double y {};
	double width {};
	double height {};
	std::string tag;
};

//OTIO can't handle float, so use double
using Details = std::vector<std::variant<OVIRect, OVIRectTag, double, bool>>;

struct Outcome {
	bool detect {};
	Details list;
};

class IPluginProcess : public IPlugin
{
public:
	~IPluginProcess() override = default;

	virtual Outcome process(ovi::FramePack* frame) = 0;
};

#endif /* __OPEN_VIDEO_INTELLIGENCE_IPLUGIN_PROCESS_H__ */
