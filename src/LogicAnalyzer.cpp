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

#include <memory>

#include "LogicAnalyzer.h"
#include "Log.h"
#include "Exception.h"

using namespace ovi;

#define OVI_EOPP "OVI_EOPP"  // End Of Plugin Pipeline

typedef enum {
	LOGICAL_OPERATOR_NONE = 0,
	LOGICAL_OPERATOR_AND,
	LOGICAL_OPERATOR_OR,
	LOGICAL_OPERATOR_COLON,
} LogicalOperator;

static bool isLogicalOperator(const std::string& str)
{
	return (str == OVI_OP_AND || str == OVI_OP_OR ||  str == OVI_OP_COLON);
}

static bool equalMetaForm(const MetaForm& a, const MetaForm& b)
{
	return (a == b || a == METAFORM_ANY || b == METAFORM_ANY);
}

static bool isEffectType(const Plugin& plugin)
{
	return (plugin.type == PLUGIN_TYPE_VIDEO_EFFECT || plugin.type == PLUGIN_TYPE_AUDIO_EFFECT);
};

bool ovi::validate_logic(const std::vector<std::string>& request, const PluginManager* pluginManager)
{
	LOG_ENTER();

	if (request.empty()) {
		LOG_ERROR("empty");
		return false;
	}

	constexpr size_t limit { 1000 };
	if (request.size() > limit) {
		LOG_ERROR("size is over");
		return false;
	}

	// It should start with a process or logical not at the begin.
	std::string begin = request.front();
	std::string end = request.back();
	if (isLogicalOperator(begin)) {
		LOG_ERROR("Begin: %s", begin.c_str());
		return false;
	}

	// It should finish with a process at the end.
	if (isLogicalOperator(end) || (end == OVI_OP_UNCUT)) {
		LOG_ERROR("End: %s", end.c_str());
		return false;
	}

	std::string prev;
	bool validate_uncut = true;
	for (const auto& str : request) {
		if (isLogicalOperator(str)) {
			// The operator should not come out continuously.
			if (isLogicalOperator(prev)) {
				LOG_ERROR("%s: %s", prev.c_str(), str.c_str());
				return false;
			}
			if ((!validate_uncut) && (str != OVI_OP_COLON))
				break;
			prev = str;
			continue;
		}

		// Before a process, it should be operator.
		if (!prev.empty() &&
			!isLogicalOperator(prev) &&
			(prev != OVI_OP_UNCUT)) {
			LOG_ERROR("%s: %s", prev.c_str(), str.c_str());
			return false;
		}

		if (str == OVI_OP_UNCUT) {
			LOG_DEBUG("%s: %s", prev.c_str(), str.c_str());
			if (!validate_uncut)
				break;
			prev = str;
			validate_uncut = false;
			continue;
		}

		if (!pluginManager->exist(str)) {
			LOG_ERROR("%s: %s", prev.c_str(), str.c_str());
			return false;
		}

		if (prev == OVI_OP_COLON &&
			!isEffectType(pluginManager->find(str))) {
			LOG_ERROR("%s: %s", prev.c_str(), str.c_str());
			return false;
		}

		if (isEffectType(pluginManager->find(str)))
			validate_uncut = true;

		prev = str;
	}

	if (!validate_uncut) {
		LOG_ERROR("Invalid uncut request!");
		return false;
	}

	return true;
}

bool ovi::validate_link(const LogicAnalyzer* logicAnalyzer, const PluginManager* pluginManager, const std::string& renderId)
{
	LOG_ENTER();

	std::string prev;
	std::string detect;
	for (const auto& str : logicAnalyzer->expression()) {
		if (isLogicalOperator(str)) {
			detect = prev;
			prev = str;
			continue;
		}

		if (prev != OVI_OP_COLON &&
			!(prev == OVI_OP_AND && isEffectType(pluginManager->find(str)))) {
			prev = str;
			continue;
		}

		const std::string& effectName = pluginManager->getAttr(str, "name");
		MetaForm detectMetaForm = pluginManager->getMetaForm(detect, {});
		MetaForm effectMetaForm = pluginManager->getMetaForm(renderId, effectName);
		if (!equalMetaForm(detectMetaForm, effectMetaForm)) {
			LOG_ERROR("[%s] metaForm: %d", detect.c_str(), detectMetaForm);
			LOG_ERROR("[%s] metaForm: %d  effect: %s", renderId.c_str(), effectMetaForm, effectName.c_str());
			return false;
		}

		prev = str;
	}

	return true;
}

PluginNode::PluginNode(const std::string& id, bool cut)
	: _include(true), _cut(cut)
{
	_plugins.push_back(id);
}

PluginNode::~PluginNode()
{
}

std::string PluginNode::pop()
{
	if (!_include)
		return {};
	if (_pos >= _plugins.size())
		return {};
	return _plugins[_pos++];
}

bool PluginNode::post(bool include)
{
	if (_pos == 1)
		_include = include;
	if (!_cut)
		return true;
	return include;
}

// LCOV_EXCL_START
void PluginNode::dump()
{
	for (const auto& plugin : _plugins) {
		if (plugin == _plugins[0])
			std::cout << plugin << info() << ":";
		else
			std::cout << plugin << ":";
	}
}
// LCOV_EXCL_STOP

PluginPipeline::PluginPipeline()
{
}

PluginPipeline::~PluginPipeline()
{
}

void PluginPipeline::reset()
{
	_pos = 0;
	for (auto& node : _pipeline)
		node->reset();
}

void PluginPipeline::push(PluginNodePtr plugin)
{
	_pipeline.push_back(plugin);
}

std::string PluginPipeline::pop(bool include)
{
	// check if it will move in the pipeline
	if (!include) {
		LOG_DEBUG("include %d", include);
		return {};
	}

	// get current node
	std::string plugin = _pipeline[_pos]->pop();
	if (!plugin.empty())
		return plugin;

	// if the current node is empty, goes next node
	if (++_pos >= _pipeline.size())
		return OVI_EOPP;

	return _pipeline[_pos]->pop();
}

PluginNodePtr PluginPipeline::current() const
{
	if (_pos >= _pipeline.size())
		return nullptr;
	return _pipeline[_pos];
}

void PluginPipeline::setEssential()
{
	if (_pipeline.empty())
		return ;
	if (_pipeline.size() != 1)
		return ;

	_essential = true;
}

// LCOV_EXCL_START
void PluginPipeline::dump() const
{
	std::cout << (_essential ? "[Essential]" : "[No Essential]");
	for (auto& node : _pipeline) {
		node->dump();
		std::cout << " ";
	}
}
// LCOV_EXCL_STOP

LogicAnalyzer::LogicAnalyzer(const std::vector<std::string>& expression, const PluginManager* pluginManager)
	: _expression(expression)
{
	LOG_ENTER();

	runAnalysis(expression, pluginManager);
}

LogicAnalyzer::~LogicAnalyzer()
{
}

void LogicAnalyzer::reset()
{
	LOG_ENTER();
	_pos = 0;
	for (auto& pipeline : _pipelines)
		pipeline->reset();
}

std::string LogicAnalyzer::nextPlugin(bool include)
{
	PluginNodePtr currentNode = _pipelines[_pos]->current();

	// process 'cut'
	_include = currentNode->post(include);

	std::string next = _pipelines[_pos]->pop(_include);
	// if all of a pipeline are included, it end.
	if (next == OVI_EOPP) {
		for (_pos = _pos + 1; _pos < _pipelines.size(); _pos++) {
			if (_pipelines[_pos]->isEssential())
				break;
		}
		if (_pos >= _pipelines.size())
			return OVI_EOP;
		_pipelines[_pos]->reset();
		return nextPlugin(true);
	}

	// if next is empty, move to another pipeline.
	if (next.empty()) {
		if (++_pos >= _pipelines.size())
			return OVI_EOP;
		_pipelines[_pos]->reset();
		return nextPlugin(true);
	}

	LOG_DEBUG("next is %s in case of %d ", next.c_str(), _include);
	return next;
}

void LogicAnalyzer::runAnalysis(std::vector<std::string> expression, const PluginManager* pluginManager)
{
	bool cut = true;
	LogicalOperator logicalOperator = LOGICAL_OPERATOR_NONE;
	PluginNodePtr pluginNode = nullptr;
	PluginPipelinePtr pipeline = std::make_shared<PluginPipeline>();
	_pipelines.push_back(pipeline);

	LOG_ENTER();

	auto getLogicalOperator = [](const std::string& str) -> LogicalOperator {
		if (str == OVI_OP_AND)
			return LOGICAL_OPERATOR_AND;
		if (str == OVI_OP_OR)
			return LOGICAL_OPERATOR_OR;
		if (str == OVI_OP_COLON)
			return LOGICAL_OPERATOR_COLON;
		return LOGICAL_OPERATOR_NONE;
	};

	for (auto& str : expression) {
		if (str == OVI_OP_UNCUT) {
			cut = false;
			continue;
		}

		if (isLogicalOperator(str)) {
			logicalOperator = getLogicalOperator(str);
			continue;
		}

		switch(logicalOperator) {
		case LOGICAL_OPERATOR_AND:
			// Add new plugin node at whole pipelines.
			pluginNode = std::make_shared<PluginNode>(str, cut);
			for (auto& _pipeline : _pipelines)
				_pipeline->push(pluginNode);
			break;
		case LOGICAL_OPERATOR_OR:
			// Add new pipeline and insert new plugin node at new pipeline.
			pipeline = std::make_shared<PluginPipeline>();
			_pipelines.push_back(pipeline);
			pluginNode = std::make_shared<PluginNode>(str, cut);
			pipeline->push(pluginNode);
			break;
		case LOGICAL_OPERATOR_COLON:
			// Add new plugin at the current node and set essential pipeline.
			pluginNode->push(str);
			if (_pipelines.size() > 1)
				pipeline->setEssential();
			break;
		default:
			// Add new plugin as default.
			pluginNode = std::make_shared<PluginNode>(str, cut);
			pipeline->push(pluginNode);
			break;
		}
		logicalOperator = LOGICAL_OPERATOR_NONE;
		cut = true;
	}
	// all '_pos' to '0'
	reset();
	// dump();
}

// LCOV_EXCL_START
void LogicAnalyzer::dump() const
{
	LOG_ENTER();
	size_t index = 0;

	std::cout << "[LogicAnalyzer::Dump::Start]" << std::endl;
	for (const auto& pipeline : _pipelines) {
		std::cout << "[PIPELINE:" << std::to_string(index++) << "]";
		pipeline->dump();
		std::cout << std::endl;
	}
	std::cout << "[LogicAnalyzer::Dump::End]" << std::endl;
}
// LCOV_EXCL_STOP
