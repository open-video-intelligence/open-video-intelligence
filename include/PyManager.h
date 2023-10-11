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

#ifndef __OPEN_VIDEO_INTELLIGENCE_PY_MANAGER_H__
#define __OPEN_VIDEO_INTELLIGENCE_PY_MANAGER_H__

#ifdef OVI_ENABLE_PYTHON

#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <chrono>
#include <future>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "PluginManager.h"
#include "ThreadRunner.h"

namespace ovi {

using ResponseData = std::variant<PluginInfo, Outcome, bool>;

class PyManager : public ThreadRunner
{
public:
	PyManager();
	~PyManager();

	PluginInfo getPluginInfo(const std::string& moduleName);
	int create(const std::string& moduleName);
	void remove(int key);
	int setAttributes(int key, std::map<std::string, std::string> attrs);
	Outcome process(int key, FramePack* frame);

private:
	PyObject* find(int key);

	enum request {
		PY_INFO,
		PY_CREATE,
		PY_REMOVE,
		PY_ATTRS,
		PY_PROC,
	};

	struct QueueData {
		request type {};
		int key {};
		std::string moduleName;
		std::map<std::string, std::string> attrs;
		FramePack* frame {};
		std::promise<ResponseData>* response;
	};

	void worker() override;
	void pyGetPluginInfo(const QueueData& data);
	void pyCreate(const QueueData& data);
	void pyDelete(const QueueData& data);
	void pySetAttrs(const QueueData& data);
	void pyProcess(const QueueData& data);

	std::map<int, PyObject*> _modules;

	std::queue<QueueData> _req {};
	std::mutex _m {};
};

}

#endif /* OVI_ENABLE_PYTHON */

#endif /* __OPEN_VIDEO_INTELLIGENCE_PY_MANAGER_H__ */
