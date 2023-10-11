/**
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


#include "Callback.h"
#include "Log.h"

using namespace ovi;

ErrorCallback::ErrorCallback(void* handle, ovi_error_cb cb, void* userData)
							: AbstractCallback(handle, userData), _callback(cb)
{
	LOG_INFO(">>> callback %p, handle %p, userData %p registered",
			reinterpret_cast<void*>(cb), handle, userData);
}

void ErrorCallback::invoke(VariantData data)
{
	auto error = std::get<ovi_error_e>(data);

	LOG_INFO(">>> callback %p, handle %p, error %d, userData %p",
			reinterpret_cast<void*>(_callback), _handle, error, _userData);

	_callback(_handle, error, _userData);
}

ProgressCallback::ProgressCallback(void* handle, ovi_progress_cb cb, void* userData)
							: AbstractCallback(handle, userData), _callback(cb)
{
	LOG_INFO(">>> callback %p, handle %p, userData %p registered",
			reinterpret_cast<void*>(cb), handle, userData);
}

void ProgressCallback::invoke(VariantData data)
{
	auto progress = std::get<std::string>(data);

	LOG_INFO(">>> callback %p, handle %p, progress %s, userData %p",
			reinterpret_cast<void*>(_callback), _handle, progress.c_str(), _userData);

	_callback(_handle, progress.c_str(), _userData);
}

StateChangedCallback::StateChangedCallback(void* handle, ovi_state_changed_cb cb, void* userData)
							: AbstractCallback(handle, userData), _callback(cb)
{
	LOG_INFO(">>> callback %p, handle %p, userData %p registered",
			reinterpret_cast<void*>(cb), handle, userData);
}

void StateChangedCallback::invoke(VariantData data1, VariantData data2)
{
	auto previous = std::get<ovi_state_e>(data1);
	auto current = std::get<ovi_state_e>(data2);

	LOG_INFO(">>> StateChangedCallback %p, handle %p, previous %d, current %d, userData %p",
			reinterpret_cast<void*>(_callback), _handle, previous, current, _userData);

	_callback(_handle, previous, current, _userData);
}
