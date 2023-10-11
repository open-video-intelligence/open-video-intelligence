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


#include "FramePackerFactory.h"
#include "FramePackerFFMPEG.h"
#include "Exception.h"

using namespace ovi;

IFramePackerPtr FramePackerFactory::create(MediaType type)
{
	switch (type) {
	case MEDIA_TYPE_VIDEO:
		return IFramePackerPtr(new FramePackerAvVideo());

	case MEDIA_TYPE_AUDIO:
		return IFramePackerPtr(new FramePackerAvAudio());

	default:
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "Invalid type");
	}
}