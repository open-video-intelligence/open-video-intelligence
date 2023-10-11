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

#ifndef __OPEN_VIDEO_INTELLIGENCE_LOGIC_ANALYZER_H__
#define __OPEN_VIDEO_INTELLIGENCE_LOGIC_ANALYZER_H__

#include "PluginManager.h"

namespace ovi {

#define OVI_EOP "OVI_EOP"  // End of Plugin

class LogicAnalyzer;

bool validate_logic(const std::vector<std::string>& request, const PluginManager* pluginManager);
bool validate_link(const LogicAnalyzer* logicAnalyzer, const PluginManager* pluginManager, const std::string& renderId);

class PluginNode;
class PluginPipeline;
using PluginNodePtr = std::shared_ptr<PluginNode>;
using PluginPipelinePtr = std::shared_ptr<PluginPipeline>;

class PluginNode
{
public:
	explicit PluginNode(const std::string& id, bool cut = true);
	~PluginNode();

	void reset() { _pos = 0; _include = true; }
	bool post(bool include);
	void push(const std::string& plugin) { _plugins.push_back(plugin); }
	void dump();

	std::string pop();

private:
	// LCOV_EXCL_START
	inline std::string boolToString(bool cond, const std::string& str) {
		return ((cond) ? str : ("not " + str));
	}
	inline std::string info() {
		return ("(" + boolToString(_cut, "cut") + ")");
	}
	// LCOV_EXCL_STOP

	std::vector<std::string> _plugins {};
	size_t _pos {}; // point to the current plugin
	bool _include;
	bool _cut;
};

class PluginPipeline
{
public:
	PluginPipeline();
	~PluginPipeline();

	void reset();
	void push(PluginNodePtr plugin);
	std::string pop(bool include);
	PluginNodePtr current() const;
	void setEssential();
	bool isEssential() const { return _essential; }
	void dump() const;

private:
	std::vector<PluginNodePtr> _pipeline {};
	size_t _pos {}; // point to the current node
	bool _essential {};
};

class LogicAnalyzer
{
public:
	LogicAnalyzer(const std::vector<std::string>& expression, const PluginManager* pluginManager);
	~LogicAnalyzer();

	void reset();
	std::string nextPlugin(bool result);
	bool include() const { return _include; }
	const std::vector<std::string>& expression() const { return _expression; }

private:
	void runAnalysis(std::vector<std::string> expression, const PluginManager* pluginManager);
	void dump() const;

	std::vector<std::string> _expression {};
	std::vector<PluginPipelinePtr> _pipelines;
	size_t _pos {}; // point to the current pipeline
	bool _include {};
};

} // ovi

#endif // __OPEN_VIDEO_INTELLIGENCE_LOGIC_ANALYZER_H__
