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

#ifdef OVI_ENABLE_PYTHON

#include "PyManager.h"
#include "Log.h"
#include "Exception.h"
#include "IPluginProcess.h"
#include "Types.h"

using namespace ovi;


PyManager::PyManager()
	: ThreadRunner()
{
	start();
}

PyManager::~PyManager()
{
}

PyObject* PyManager::find(int key)
{
	auto iter = _modules.find(key);
	if (iter == _modules.end())
		throw Exception(OVI_ERROR_INVALID_PARAMETER, "No key");

	return iter->second;
}

PluginInfo PyManager::getPluginInfo(const std::string& moduleName)
{
	std::unique_lock<std::mutex> locker(_m);

	std::promise<ResponseData> response;
	auto f = response.get_future();

	_req.push(QueueData{
		.type = PY_INFO,
		.moduleName = moduleName,
		.response = &response
	});

	locker.unlock();

	return std::get<PluginInfo>(f.get());
}

int PyManager::create(const std::string& moduleName)
{
	static int key = 0;

	std::lock_guard<std::mutex> locker(_m);

	_req.push(QueueData{
		.type = PY_CREATE,
		.key = ++key,
		.moduleName = moduleName,
	});

	return key;
}

void PyManager::remove(int key)
{
	std::lock_guard<std::mutex> locker(_m);

	_req.push(QueueData{
		.type = PY_REMOVE,
		.key = key,
	});
}

int PyManager::setAttributes(int key, std::map<std::string, std::string> attrs)
{
	std::unique_lock<std::mutex> locker(_m);

	std::promise<ResponseData> response;
	auto f = response.get_future();

	_req.push(QueueData{
		.type = PY_ATTRS,
		.key = key,
		.attrs = attrs,
		.response = &response
	});

	locker.unlock();

	return std::get<bool>(f.get()) ? OVI_ERROR_NONE : OVI_ERROR_INVALID_PARAMETER;
}

Outcome PyManager::process(int key, FramePack* frame)
{
	std::unique_lock<std::mutex> locker(_m);

	std::promise<ResponseData> response;
	auto f = response.get_future();

	_req.push(QueueData{
		.type = PY_PROC,
		.key = key,
		.frame = frame,
		.response = &response
	});

	locker.unlock();

	return std::get<Outcome>(f.get());
}

void PyManager::worker()
{
	Py_Initialize();
	char import[100];
	snprintf(import, sizeof(import), "import sys; sys.path.append('%s')", PLUGIN_INSTALLED_DIR);
	PyRun_SimpleString(import);

	while (_run.load()) {
		std::unique_lock<std::mutex> locker(_m);

		if (_req.empty()) {
			locker.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		auto data = _req.front();
		_req.pop();

		switch (data.type) {
		case PY_INFO:
			pyGetPluginInfo(data);
			break;

		case PY_CREATE:
			pyCreate(data);
			break;

		case PY_REMOVE:
			pyDelete(data);
			break;

		case PY_ATTRS:
			pySetAttrs(data);
			break;

		case PY_PROC:
			pyProcess(data);
			break;

		default:
			//Do nothing..
			break;
		}
	}
	/* Do not use spdlog here */
	for (auto& it : _modules)
		Py_DECREF(it.second);

	_modules.clear();
	Py_FinalizeEx();
}

static const char* _getStringForPy(PyObject* mod, const std::string& funcName)
{
	auto func = PyObject_GetAttrString(mod, funcName.c_str());
	auto value = PyObject_CallObject(func, nullptr);

	auto res = PyUnicode_AsUTF8(value);

	Py_DECREF(value);
	Py_DECREF(func);

	return res;
}

static int _getIntForPy(PyObject* mod, const std::string& funcName)
{
	auto func = PyObject_GetAttrString(mod, funcName.c_str());
	auto value = PyObject_CallObject(func, nullptr);

	auto res = static_cast<int>(PyLong_AsLong(value));

	Py_DECREF(value);
	Py_DECREF(func);

	return res;
}

static std::vector<int> _getIntListForPy(PyObject* mod, const std::string& funcName)
{
	auto func = PyObject_GetAttrString(mod, funcName.c_str());
	auto value = PyObject_CallObject(func, nullptr);

	std::vector<int> res;

	for (int i = 0; i < PyList_Size(value); ++i) {
		auto item = PyList_GetItem(value, i);
		res.push_back(static_cast<int>(PyLong_AsLong(item)));
	}

	Py_DECREF(value);
	Py_DECREF(func);

	return res;
}

static std::vector<Attribute> _getListForPy(PyObject* mod, const std::string& funcName)
{
	auto func = PyObject_GetAttrString(mod, funcName.c_str());
	auto value = PyObject_CallObject(func, nullptr);

	std::vector<Attribute> res;

	for (int i = 0; i < PyList_Size(value); ++i) {
		auto item = PyList_GetItem(value, i);
		auto key = PyUnicode_AsUTF8(PyTuple_GetItem(item, 0));
		auto type = PyUnicode_AsUTF8(PyTuple_GetItem(item, 1));
		auto desc = PyUnicode_AsUTF8(PyTuple_GetItem(item, 2));
		res.push_back({
			std::string(key), std::string(type), std::string(desc)
		});
	}

	Py_DECREF(value);
	Py_DECREF(func);

	return res;
}

void PyManager::pyGetPluginInfo(const QueueData& data)
{
	auto name = PyUnicode_DecodeFSDefault(data.moduleName.c_str());
	auto mod = PyImport_Import(name);
	if (!mod) {
		LOG_WARN("Import python module failed");
		data.response->set_value({});
		return;
	}
	Py_DECREF(name);

	auto attrs = _getListForPy(mod, "pluginAttributeList");

	PluginInfo info { LANG_PYTHON,
						_getStringForPy(mod, "pluginName"),
						static_cast<PluginType>(_getIntForPy(mod, "pluginType")),
						_getIntListForPy(mod, "pluginFormat"),
						static_cast<MetaForm>(_getIntForPy(mod, "pluginMetaForm")),
						_getStringForPy(mod, "pluginDescription"),
						data.moduleName,
						attrs };

	data.response->set_value(info);

	Py_DECREF(mod);
}

void PyManager::pyCreate(const QueueData& data)
{
	auto name = PyUnicode_DecodeFSDefault(data.moduleName.c_str());
	auto mod = PyImport_Import(name);
	Py_DECREF(name);

	_modules.insert({
		data.key, mod
	});
}

void PyManager::pyDelete(const QueueData& data)
{
	try {
		auto mod = find(data.key);
		Py_DECREF(mod);
		_modules.erase(data.key);
	} catch (const Exception& e) {
		LOG_ERROR("No item");
	}
}

void PyManager::pySetAttrs(const QueueData& data)
{
	try {
		auto mod = find(data.key);
		auto func = PyObject_GetAttrString(mod, "setAttrs");

		if (func == nullptr) {
			LOG_ERROR("Get function failed");

			data.response->set_value(false);
			return;
		}

		auto args = PyTuple_New(1);
		auto dict = PyDict_New();
		for (const auto& item : data.attrs)
			PyDict_SetItemString(dict, item.first.c_str(), PyUnicode_FromString(item.second.c_str()));

		PyTuple_SetItem(args, 0, dict);

		auto value = PyObject_CallObject(func, args);
		auto res = static_cast<int>(PyLong_AsLong(value));

		data.response->set_value(res == 0);

		Py_DECREF(dict);
		Py_DECREF(args);
		Py_DECREF(value);
		Py_DECREF(func);
	} catch (const Exception& e) {
		LOG_ERROR("No item");
		data.response->set_value(false);
	}
}

void PyManager::pyProcess(const QueueData& data)
{
	Outcome o;
	try {
		auto mod = find(data.key);
		PyObject* func = PyObject_GetAttrString(mod, "process");
		if (func == nullptr) {
			LOG_ERROR("Get function failed");
			data.response->set_value(o);
			return;
		}

		if (data.frame->type() == MEDIA_TYPE_VIDEO) {
			ovi::VideoFramePack* vFrame = dynamic_cast<ovi::VideoFramePack*>(data.frame);
			assert(vFrame);
			int width {};
			int height {};
			std::tie(width, height, std::ignore) = vFrame->videoProperties();

			auto args = Py_BuildValue("(iiy#Lf)",
				width, height, vFrame->data(), vFrame->size(), vFrame->duration(), vFrame->framerate());
			auto value = PyObject_CallObject(func, args);
			Py_DECREF(args);

			//TODO: Need to check value type before casting..
			if (PyTuple_Check(value)) {
				o.detect = PyLong_AsLong(PyTuple_GET_ITEM(value, 0));
				auto list = PyTuple_GET_ITEM(value, 1);
				size_t len = PyList_GET_SIZE(list);

				for (size_t i = 0; i < len; i++) {
					auto item = PyList_GET_ITEM(list, i);
					if (PyTuple_Check(item)) {
						auto x = PyLong_AsLong(PyTuple_GET_ITEM(item, 0));
						auto y = PyLong_AsLong(PyTuple_GET_ITEM(item, 1));
						auto w = PyLong_AsLong(PyTuple_GET_ITEM(item, 2));
						auto h = PyLong_AsLong(PyTuple_GET_ITEM(item, 3));

						OVIRect r {
							static_cast<double>(x),
							static_cast<double>(y),
							static_cast<double>(w),
							static_cast<double>(h),
						};
						o.list.push_back(r);
					} else if (PyBool_Check(item)) {
						bool v = PyObject_IsTrue(item);
						o.list.push_back(v);
					}
				}
			}

			Py_DECREF(value);
			Py_DECREF(func);

		} else if (data.frame->type() == MEDIA_TYPE_AUDIO) {
			ovi::AudioFramePack* aFrame = dynamic_cast<ovi::AudioFramePack*>(data.frame);
			assert(aFrame);
			auto [channels, samplerate, format, samples] = aFrame->audioProperties();

			auto args = Py_BuildValue("(iiiiy#)", channels, samplerate, format, samples, aFrame->data(), aFrame->size());
			auto value = PyObject_CallObject(func, args);
			Py_DECREF(args);
			if (PyTuple_Check(value)) {
				o.detect = PyLong_AsLong(PyTuple_GET_ITEM(value, 0));
				auto db = PyFloat_AsDouble(PyTuple_GET_ITEM(value, 1));
				o.list.push_back(db);
			}

			Py_DECREF(value);
			Py_DECREF(func);
		}

		data.response->set_value(o);
	} catch (const Exception& e) {
		LOG_ERROR("No item");
		data.response->set_value(o);
	}
}

#endif /* OVI_ENABLE_PYTHON */