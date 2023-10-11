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

#ifndef __OPEN_VIDEO_INTELLIGENCE_TYPES_H__
#define __OPEN_VIDEO_INTELLIGENCE_TYPES_H__

namespace ovi {

/**
 * @brief Enumeration for plugin type.
 * @remarks This enumeration should be matched with #ovi_plugin_type_e
 */
typedef enum {
	PLUGIN_TYPE_NONE,
	PLUGIN_TYPE_VIDEO_DETECT,
	PLUGIN_TYPE_VIDEO_EFFECT,
	PLUGIN_TYPE_AUDIO_DETECT,
	PLUGIN_TYPE_AUDIO_EFFECT,
	PLUGIN_TYPE_RENDER,
} PluginType;

/**
 * @brief Enumeration for media type.
 */
typedef enum {
	MEDIA_TYPE_NONE,
	MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_VIDEO,
} MediaType;

/**
 * @brief Enumeration for video format.
 */
typedef enum {
	VIDEO_FORMAT_NONE,
	VIDEO_FORMAT_YUV420P,
	VIDEO_FORMAT_YUV422P,
	VIDEO_FORMAT_RGB24,
	VIDEO_FORMAT_BGR24,
	VIDEO_FORMAT_NV12,
	VIDEO_FORMAT_NV21,
	VIDEO_FORMAT_ARGB,
	VIDEO_FORMAT_RGBA,
	VIDEO_FORMAT_ABGR,
	VIDEO_FORMAT_BGRA,
	VIDEO_FORMAT_GRAY8,
	VIDEO_FORMAT_MAX,
} VideoFormat;

/**
 * @brief Enumeration for audio format.
 */
typedef enum {
	AUDIO_FORMAT_NONE,
	AUDIO_FORMAT_U8,	/**< unsigned 8 bits */
	AUDIO_FORMAT_S16,	/**< signed 16 bits */
	AUDIO_FORMAT_S32,	/**< signed 32 bits */
	AUDIO_FORMAT_FLT,	/**< float */
	AUDIO_FORMAT_DBL,	/**< double */
	AUDIO_FORMAT_S64,	/**< signed 64 bits */

	AUDIO_FORMAT_U8P,	/**< unsigned 8 bits, planar */
	AUDIO_FORMAT_S16P,	/**< signed 16 bits, planar */
	AUDIO_FORMAT_S32P,	/**< signed 32 bits, planar */
	AUDIO_FORMAT_FLTP,	/**< float, planar */
	AUDIO_FORMAT_DBLP,	/**< double, planar */
	AUDIO_FORMAT_S64P,	/**< signed 64 bits, planar */
	AUDIO_FORMAT_MAX,
} AudioFormat;

}

#endif // __OPEN_VIDEO_INTELLIGENCE_TYPES_H__