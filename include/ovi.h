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

#ifndef __OVI_INTERFACE_H__
#define __OVI_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include "ovi_types.h"

typedef void *session;

/* plugin */
/**
 * @brief Iterates through the available plugins in the system.
 *
 * @param[in] type the type of plugin to find, OVI_PLUGIN_TYPE_NONE to find all types
 * @param[in] name the name of plugin to find, nullptr to find all
 * @param[in] callback the callback function to be invoked
 * @param[in] user_data the user data to be passed to the callback function
 * @return int 0 on success
 */
int ovi_available_plugin_foreach(ovi_plugin_type_e type, const char *name, plugin_foreach_cb callback, void *user_data);

/**
 * @brief Iterates through the attributes for the plugin.
 *
 * @param[in] name the name of plugin to get attributes
 * @param[in] callback the callback function to be invoked
 * @param[in] user_data the user data to be passed to the callback function
 * @return int 0 on success
 */
int ovi_available_plugin_foreach_attribute(const char *name, plugin_attribute_foreach_cb callback, void *user_data);

/* session : basic operation */
/**
 * @brief Creates session.
 *
 * @param[out] s the handle pointer to be created
 * @return int 0 on success
 */
int ovi_session_create(session *s);

/**
 * @brief Starts session.
 *
 * @param[in] s the session handle
 * @return int 0 on success
 */
int ovi_session_start(session s);

/**
 * @brief Stops session.
 *
 * @param[in] s the session handle
 * @return int 0 on success
 */
int ovi_session_stop(session s);

/**
 * @brief Gets the current state of a session.
 *
 * @param[in] s the session handle
 * @param[out] state the state of session
 * @return int 0 on success
 */
int ovi_session_get_state(session s, ovi_state_e *state);

/**
 * @brief Deinitializes and frees session.
 *
 * @param[in] s the session handle
 * @return int 0 on success
 */
int ovi_session_destroy(session s);

/* session : plugin manipulation */
/**
 * @brief Adds a plugin into the session.
 *
 * @param[in] s the session handle
 * @param[in] name the name of the plugin to add
 * @param[out] uid the uid of the added plugin
 * @return int 0 on success
 */
int ovi_session_add_plugin(session s, const char *name, const char **uid);

/**
 * @brief Sets the attribute for the plugin.
 *
 * @param[in] s the session handle
 * @param[in] uid the uid of plugin
 * @return int 0 on success
 */
int ovi_session_set_plugin_attribute(session s, const char *uid, const char *first_property_name, ...);

/**
 * @brief Links plugins and operators.
 *
 * @param[in] s the session handle
 * @param[in] uid the uid of the first plugin
 * @param[in] ... list of plugins and operators
 * @return int 0 on success
 */
int ovi_session_link_plugins(session s, const char *uid, ...);

/**
 * @brief Links plugins and operators array.
 *
 * @param[in] s the session handle
 * @param[in] items items of plugin and operator
 * @param[in] size the size of items
 * @return int 0 on success
 */
int ovi_session_link_plugins_with_list(session s, const char *items[], unsigned int size);

/* session : mandatory setting */
/**
 * @brief Sets the media path to operate the session.
 *
 * @param[in] s the session handle
 * @param[in] media_path the media path
 * @return int 0 on success
 */
int ovi_session_set_media_path(session s, const char *media_path);

/**
 * @brief Sets the render and the path to store the result of rendering.
 *
 * @param[in] s the session handle
 * @param[in] render_name the render name
 * @param[in] output_path the path to store the result
 * @return int 0 on success
 */
int ovi_session_set_render(session s, const char *render_name, const char *output_path);

/* session : callbacks */
/**
 * @brief Sets the callback function to be invoked when the error occur.
 *
 * @param[in] s the session handle
 * @param[in] callback the callback function to be invoked
 * @param[in] user_data the user data to be passed to the callback function
 * @return int 0 on success
 */
int ovi_session_set_error_cb(session s, ovi_error_cb callback, void *user_data);

/**
 * @brief Unsets the callback function to be invoked when the error occur.
 *
 * @param[in] s the session handle
 * @return int 0 on success
 */
int ovi_session_unset_error_cb(session s);

/**
 * @brief Sets the callback function to be invoked in progress.
 *
 * @param[in] s the session handle
 * @param[in] callback the callback function to be invoked
 * @param[in] user_data the user data to be passed to the callback function
 * @return int 0 on success
 */
int ovi_session_set_progress_cb(session s, ovi_progress_cb callback, void *user_data);

/**
 * @brief Unsets the callback function to be invoked in progress.
 *
 * @param[in] s the session handle
 * @return int 0 on success
 */
int ovi_session_unset_progress_cb(session s);

/**
 * @brief Sets the callback function to be invoked when the state is changed.
 *
 * @param[in] s the session handle
 * @param[in] callback the callback function to be invoked
 * @param[in] user_data the user data to be passed to the callback function
 * @return int 0 on success
 */
int ovi_session_set_state_changed_cb(session s, ovi_state_changed_cb callback, void *user_data);

/**
 * @brief Unsets a callback function to be invoked when the state is changed.
 *
 * @param[in] s the session handle
 * @return int 0 on success
 */
int ovi_session_unset_state_changed_cb(session s);

/* session : optional */
/**
 * @brief Sets the skip_frames for analyzing.
 *
 * @param[in] s the session handle
 * @param[in] skip_frames the number of video frames to skip
 * @return int 0 on success
 *
 * To speed up the analysis, you can adjust the number of video frames to analyze.
 * 1 means that one frame is analyzed and the next frame is skipped
 * 2 means that one frame is analyzed and the next two frames are skipped.
 * The skip_frames are treated as having the same result as the analyzed frame result.
 * The default is 0, which analyzes all frames without skip, and the user can set values from 0 to 5.
 */
int ovi_session_set_skip_video_frames(session s, size_t skip_frames);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OVI_INTERFACE_H__ */
