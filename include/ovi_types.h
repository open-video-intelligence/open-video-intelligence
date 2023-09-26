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

#ifndef __OVI_TYPES_H__
#define __OVI_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>

/**
 * @brief Logical operators for links.
 */
#define OVI_OP_AND "&"
#define OVI_OP_OR "|"
#define OVI_OP_COLON ":"
#define OVI_OP_UNCUT "~"

/**
 * @brief Enumeration for error.
 */
typedef enum {
	OVI_ERROR_NONE = 0,
	OVI_ERROR_INVALID_PARAMETER = -1,
	OVI_ERROR_INVALID_OPERATION = -2,
	OVI_ERROR_PERMISSION_DENIED = -3,
	OVI_ERROR_INVALID_STATE = -4,
	OVI_ERROR_NO_SUCH_FILE = -5,
	OVI_ERROR_NOT_SUPPORTED_MEDIA = -6,
	OVI_ERROR_NOT_SUPPORTED_EFFECT = -7,
	OVI_ERROR_NOT_SUPPORTED_EFFECT_ATTR = -8,
	OVI_ERROR_INVALID_EFFECT_ATTR_VALUE = -9
} ovi_error_e;

/**
 * @brief Enumeration for plugin type.
 */
typedef enum {
	OVI_PLUGIN_TYPE_NONE,
	OVI_PLUGIN_TYPE_VIDEO_DETECT,
	OVI_PLUGIN_TYPE_VIDEO_EFFECT,
	OVI_PLUGIN_TYPE_AUDIO_DETECT,
	OVI_PLUGIN_TYPE_AUDIO_EFFECT,
	OVI_PLUGIN_TYPE_RENDER,
} ovi_plugin_type_e;

/**
 * @brief Enumeration for the state of session.
 */
typedef enum {
	OVI_STATE_IDLE,	/**< Created but not started */
	OVI_STATE_ANALYSIS,
	OVI_STATE_RENDER,
} ovi_state_e;

/**
 * @brief Called when error occured
 * @remarks The callback is called in the another thread as the one that calls the API.
 * @param[in] handle the session handle
 * @param[in] error the error code
 * @param[in] user_data the user data to be passed
 */
typedef void (*ovi_error_cb)(void *handle, ovi_error_e error, void *user_data);

/**
 * @brief Called in progress
 * @remarks The callback is called in the another thread as the one that calls the API.
 * @param[in] handle the session handle
 * @param[in] progress the progress
 * @param[in] user_data the user data to be passed
 */
typedef void (*ovi_progress_cb)(void *handle, const char *progress, void *user_data);

/**
 * @brief Called when the state is changed
 * @remarks The callback is called in the another thread as the one that calls the API.
 * @param[in] handle the session handle
 * @param[in] previous the previous state
 * @param[in] current the current state
 * @param[in] user_data the user data to be passed
 */
typedef void (*ovi_state_changed_cb)(void *handle, ovi_state_e previous, ovi_state_e current, void *user_data);

/**
 * @brief Called when the available plugin is existed
 * @remarks The callback is called in the same thread as the one that calls the API.
 * @param[in] name the name of plugin
 * @param[in] type the type of pulgin
 * @param[in] description the description of plugin
 * @param[in] user_data the user data to be passed
 */
typedef bool (*plugin_foreach_cb)(const char *name, ovi_plugin_type_e type, const char *description, void *user_data);

/**
 * @brief Called when the available attribute is existed
 * @remarks The callback is called in the same thread as the one that calls the API.
 * @param[in] key the key of attribute
 * @param[in] type the type of attribute
 * @param[in] description the description of attribute
 * @param[in] user_data the user data to be passed
 */
typedef bool (*plugin_attribute_foreach_cb)(const char *key, const char *type, const char *description, void *user_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OVI_TYPES_H__ */
