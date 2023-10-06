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

#ifndef __OPEN_VIDEO_INTELLIGENCE_CALLBACK_H__
#define __OPEN_VIDEO_INTELLIGENCE_CALLBACK_H__

#include <string>
#include <variant>
#include <memory>

#include "ovi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace ovi {

using VariantData = std::variant<ovi_error_e, std::string, ovi_state_e>;

class IInvokable
{
public:
	virtual ~IInvokable() = default;

	virtual void invoke() = 0;
	virtual void invoke(VariantData data) = 0;
	virtual void invoke(VariantData data1, VariantData data2) = 0;
	// ...
};

class AbstractCallback : public IInvokable
{
public:
	AbstractCallback(void* handle, void* userData) :
		_handle(handle), _userData(userData) {}
	virtual ~AbstractCallback() = default;

// LCOV_EXCL_START
	void invoke() override {}
	void invoke(VariantData data) override {}
	void invoke(VariantData data1, VariantData data2) override {}
// LCOV_EXCL_STOP

protected:
	void* _handle {};
	void* _userData {};
};

class ErrorCallback : public AbstractCallback
{
public:
	ErrorCallback(void* handle, ovi_error_cb cb, void* userData);
	virtual ~ErrorCallback() = default;

	void invoke(VariantData data) override;

private:
	ovi_error_cb _callback {};
};

class ProgressCallback : public AbstractCallback
{
public:
	ProgressCallback(void* handle, ovi_progress_cb cb, void* userData);
	virtual ~ProgressCallback() = default;

	void invoke(VariantData data) override;

private:
	ovi_progress_cb _callback {};
};

class StateChangedCallback : public AbstractCallback
{
public:
	StateChangedCallback(void* handle, ovi_state_changed_cb cb, void* userData);
	virtual ~StateChangedCallback() = default;

	void invoke(VariantData data1, VariantData data2) override;

private:
	ovi_state_changed_cb _callback {};
};

} // ovi

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPEN_VIDEO_INTELLIGENCE_CALLBACK_H__ */
