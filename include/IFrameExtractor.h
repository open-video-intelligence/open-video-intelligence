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

#ifndef __OPEN_VIDEO_INTELLIGENCE_IFRAME_EXTRACTOR_H__
#define __OPEN_VIDEO_INTELLIGENCE_IFRAME_EXTRACTOR_H__

#include "FramePack.h"

namespace ovi {

class IFrameExtractor
{
public:
	virtual ~IFrameExtractor() = default;

	virtual FramePackPtr nextVideo() const = 0;
	virtual FramePackPtr nextAudio() const = 0;

	virtual double videoFramerate() const = 0;
	virtual double audioFramerate() const = 0;
	virtual int64_t videoFrames() const = 0;
	virtual int64_t audioFrames() const = 0;
	virtual bool hasVideo() const = 0;
	virtual bool hasAudio() const = 0;
};

}

#endif // __OPEN_VIDEO_INTELLIGENCE_IFRAME_EXTRACTOR_H__
