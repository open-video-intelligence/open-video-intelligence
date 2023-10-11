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

#include <iostream>
#include <stdarg.h>

#include "ovi.h"
#include "Session.h"
#include "Log.h"

using namespace ovi;

int ovi_session_create(session *s)
{
	logger::init();
	Session* session = new Session();

	LOG_ENTER();

	*s = session;

	return OVI_ERROR_NONE;
}

int ovi_session_set_error_cb(session s, ovi_error_cb callback, void *user_data)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		session->setErrorCb(callback, user_data);
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_unset_error_cb(session s)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		session->unsetErrorCb();
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_set_progress_cb(session s, ovi_progress_cb callback, void *user_data)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		session->setProgressCb(callback, user_data);
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_unset_progress_cb(session s)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		session->unsetProgressCb();
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_set_state_changed_cb(session s, ovi_state_changed_cb callback, void *user_data)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		session->setStateChangedCb(callback, user_data);
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_unset_state_changed_cb(session s)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		session->unsetStateChangedCb();
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_add_plugin(session s, const char *name, const char **uid)
{
	LOG_ENTER();
	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	if (!name)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		*uid = session->appendPlugin(name).c_str();
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_set_plugin_attribute(session s, const char *uid, const char *first_property_name, ...)
{
	LOG_ENTER();
	if (!uid)
		return OVI_ERROR_INVALID_PARAMETER;

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	std::vector<std::string> input;
	std::map<std::string, std::string> attrs;
	va_list ap;
	va_start(ap, first_property_name);

	for (const char *val = first_property_name; val != NULL; val = va_arg(ap, const char *))
		input.push_back(val);

	va_end(ap);

	int len = input.size();

	for (int i = 0; i < len; i += 2)
		attrs.insert(std::pair<std::string, std::string>(input[i], input[i + 1]));

	try {
		session->setPluginAttrs(uid, attrs);
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}


int ovi_session_link_plugins(session s, const char *uid, ...)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	if (!uid)
		return OVI_ERROR_INVALID_PARAMETER;

	std::vector<std::string> input;
	va_list ap;

	va_start(ap, uid);

	for (const char *val = uid; val != NULL; val = va_arg(ap, const char *))
		input.push_back(val);

	va_end(ap);

	try {
		session->registerPlugin(input);
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_link_plugins_with_list(session s, const char *items[], unsigned int size)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	if (size == 0)
		return OVI_ERROR_INVALID_PARAMETER;

	std::vector<std::string> input(items, items + size);

	try {
		session->registerPlugin(input);
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_set_render(session s, const char *uid, const char *output_path)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session) {
		LOG_ERROR("invalid session");
		return OVI_ERROR_INVALID_PARAMETER;
	}

	if (!uid) {
		LOG_ERROR("invalid uid");
		return OVI_ERROR_INVALID_PARAMETER;
	}

	if (!output_path) {
		LOG_ERROR("invalid output_path");
		return OVI_ERROR_INVALID_PARAMETER;
	}

	try {
		session->setRender(uid, output_path);
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_set_media_path(session s, const char* media_path)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		session->setMediaPath(media_path);
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_get_state(session s, ovi_state_e *state)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	if (!state)
		return OVI_ERROR_INVALID_PARAMETER;

	*state = session->state();

	return OVI_ERROR_NONE;
}

int ovi_session_start(session s)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		session->start();
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_stop(session s)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		session->stop();
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_destroy(session s)
{
	LOG_ENTER();

	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	delete session;

	return OVI_ERROR_NONE;
}

int ovi_available_plugin_foreach(ovi_plugin_type_e type, const char *name, plugin_foreach_cb callback, void *user_data)
{
	LOG_ENTER();

	try {
		auto& pluginList = PluginLoader::instance().getAvailablePluginList();

		if (name) {
			for (auto& info : pluginList) {
				if (info.name.compare(name) == 0) {
					callback(info.name.c_str(), static_cast<ovi_plugin_type_e>(info.type), info.description.c_str(), user_data);
					break;
				}
			}
		} else if (type != OVI_PLUGIN_TYPE_NONE) {
			for (auto& info : pluginList) {
				if (type  == static_cast<ovi_plugin_type_e>(info.type))
					if (!callback(info.name.c_str(), static_cast<ovi_plugin_type_e>(info.type), info.description.c_str(), user_data))
						break;
			}
		} else {
			for (auto& info : pluginList)
				if (!callback(info.name.c_str(), static_cast<ovi_plugin_type_e>(info.type), info.description.c_str(), user_data))
					break;
		}
	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_available_plugin_foreach_attribute(const char *name, plugin_attribute_foreach_cb callback, void *user_data)	//ToDo
{
	LOG_ENTER();
	if (!name)
		return OVI_ERROR_INVALID_PARAMETER;

	try {
		auto& attrs = PluginLoader::instance().getPluginAttrs(name);
		if (attrs.empty()) {
			//TODO::Need to return value when attribute is empty.
			return OVI_ERROR_NONE;
		}

		for (auto& attrInfo : attrs)
			if (!callback(attrInfo.key.c_str(), attrInfo.type.c_str(), attrInfo.description.c_str(), user_data))
				break;

	} catch (const Exception& e) {
		LOG_ERROR("%s", e.what());
		return e.error();
	}

	return OVI_ERROR_NONE;
}

int ovi_session_set_skip_video_frames(session s, size_t skip_frames)
{
	auto session = static_cast<Session*>(s);
	if (!session)
		return OVI_ERROR_INVALID_PARAMETER;

	session->setSkipVideoFrames(skip_frames);

	return OVI_ERROR_NONE;
}
