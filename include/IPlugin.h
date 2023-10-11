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

#ifndef __OPEN_VIDEO_INTELLIGENCE_IPLUGIN_H__
#define __OPEN_VIDEO_INTELLIGENCE_IPLUGIN_H__

#include <iostream>
#include <map>
#include <vector>
#include "Exception.h"
#include "Types.h"

typedef enum {
	METAFORM_NONE = 0,		//No Metadata
	METAFORM_ANY,			//Any
	METAFORM_DOUBLE = 10,
	METAFORM_STRING = 20,
	METAFORM_RECT = 100,		//Rectangle (double x, double y, double w, double h)
	METAFORM_RECT_STRING,		//Rectangle, string
} MetaForm;

struct Attribute {
	std::string key;
	std::string type;
	std::string description;
};

class IPlugin
{
public:
	virtual ~IPlugin() = default;

	virtual int setAttrs(const std::map<std::string, std::string>& attrs) = 0;
};

#endif /* __OPEN_VIDEO_INTELLIGENCE_IPLUGIN_H__ */
