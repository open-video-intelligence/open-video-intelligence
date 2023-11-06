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

#include "utBase.h"
#include "LogicAnalyzer.h"
#include "PluginManager.h"
#include "ovi_types.h"


class LogicAnalyzerTest : public UtBase {
protected:
	void SetUp(void) override {
		Start();

		for (int i = 0; i < PluginNum; i++)
			_plugin[i] = _pm->load(pluginName());
		for (int i = 0; i < PluginNum; i++)
			_effect[i] = _pm->load(audioEffectPlugin());
	}

	void TearDown(void) override {
		End();
	}

	std::shared_ptr<PluginManager> _pm = std::make_shared<PluginManager>();
	static const int PluginNum = 5;
	std::string _plugin[PluginNum];
	std::string _effect[PluginNum];

	const std::string _fakePlugin = { "fake plugin" };
};

TEST_F(LogicAnalyzerTest, nextPlugin_check_return_value_if_single_plugin)
{
	std::vector<std::string> request = { _plugin[0] };
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), OVI_EOP);
}

TEST_F(LogicAnalyzerTest, nextPlugin_check_return_value_if_only_effects)
{
	std::vector<std::string> request = {
		_effect[0], ":", _effect[1], ":", _effect[2]
	};
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _effect[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _effect[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _effect[2]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), OVI_EOP);
}

TEST_F(LogicAnalyzerTest, nextPlugin_check_return_value_if_plugin_and_effect)
{
	std::vector<std::string> request = {
		_plugin[0], ":", _effect[0], ":", _effect[1], ":", _effect[2]
	};
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _effect[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _effect[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _effect[2]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), OVI_EOP);
	logicAnalyzer.reset();

	// If the detect is false, effects don't work.
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), OVI_EOP);
}

TEST_F(LogicAnalyzerTest, nextPlugin_check_return_value_if_false)
{
	std::vector<std::string> request {
		_plugin[0], OVI_OP_OR, _plugin[1], OVI_OP_AND, _plugin[2]
	};
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	// If the front is false, OVI_OP_OR goes the others.
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), OVI_EOP);
	logicAnalyzer.reset();

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[2]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), OVI_EOP);
}

TEST_F(LogicAnalyzerTest, nextPlugin_check_return_value_if_false_and_effect)
{
	std::vector<std::string> request {
		_plugin[0], OVI_OP_OR, _plugin[1], OVI_OP_AND,
		_plugin[2], ":", _effect[2]
	};
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	// If the detect is false, effects don't work.
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[2]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), OVI_EOP);
}

TEST_F(LogicAnalyzerTest, nextPlugin_check_return_value_if_true_and_effect)
{
	std::vector<std::string> request {
		_plugin[0], OVI_OP_OR, _plugin[1], ":", _effect[1]
	};
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	// Even if the _plugin[0] is true, _plugin[1]:effect[1] does work.
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _effect[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), OVI_EOP);
}

TEST_F(LogicAnalyzerTest, nextPlugin_check_return_value_if_complex_request)
{
	std::vector<std::string> request {
		_plugin[0], OVI_OP_OR, _plugin[1], OVI_OP_AND,
		_plugin[2], OVI_OP_OR, _plugin[3], OVI_OP_AND,
		_plugin[4]
	};
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[2]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[4]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), OVI_EOP);
	logicAnalyzer.reset();

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[3]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), OVI_EOP);
	logicAnalyzer.reset();

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[2]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[4]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), OVI_EOP);
	logicAnalyzer.reset();

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[3]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[4]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), OVI_EOP);
}

TEST_F(LogicAnalyzerTest, validate_logic_check_return_value)
{
	constexpr size_t limit { 10000 };
	std::vector<std::vector<std::string>> _requests {
		{ },
		{ OVI_OP_OR, _plugin[0] },
		{ OVI_OP_COLON, _effect[0] },
		{ _plugin[0], OVI_OP_AND },
		{ _plugin[0], OVI_OP_UNCUT },
		{ _plugin[0], OVI_OP_OR, OVI_OP_AND, _plugin[1] },
		{ _plugin[0], OVI_OP_AND, OVI_OP_OR, _plugin[1] },
		{ _plugin[0], OVI_OP_AND, OVI_OP_COLON, _effect[0] },
		{ _plugin[0], OVI_OP_UNCUT, OVI_OP_COLON, _effect[0] },
		{ _plugin[0], _plugin[1] },
		{ _plugin[0], OVI_OP_OR, _fakePlugin },
		{ _fakePlugin, OVI_OP_OR, _plugin[0] },
		{ OVI_OP_UNCUT, _plugin[0] },
	};

	for (auto& request : _requests)
		EXPECT_FALSE(validate_logic(request, _pm.get()));

	std::vector<std::string> requestLimit = {};
	requestLimit.resize(limit);
	EXPECT_FALSE(validate_logic(requestLimit, _pm.get()));
}

TEST_F(LogicAnalyzerTest, include_check_return_value)
{
	std::vector<std::string> request = { _plugin[0] };
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), OVI_EOP);
	EXPECT_TRUE(logicAnalyzer.include());
	logicAnalyzer.reset();

	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), OVI_EOP);
	EXPECT_FALSE(logicAnalyzer.include());
}

TEST_F(LogicAnalyzerTest, include_check_return_value_if_uncut)
{
	std::vector<std::string> request = {
		OVI_OP_UNCUT, _plugin[0], OVI_OP_COLON, _effect[0], OVI_OP_COLON, _effect[1]
	};
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	// Even if the result of a plugin[0] is 'false', the include is 'true'
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), OVI_EOP);
	EXPECT_TRUE(logicAnalyzer.include());
}

TEST_F(LogicAnalyzerTest, include_check_return_value_if_many_uncut)
{
	std::vector<std::string> request = {
		OVI_OP_UNCUT, _plugin[0], OVI_OP_COLON, _effect[0], OVI_OP_AND,
		OVI_OP_UNCUT, _plugin[1], OVI_OP_COLON, _effect[1], OVI_OP_AND,
		OVI_OP_UNCUT, _plugin[2], OVI_OP_COLON, _effect[2], OVI_OP_AND,
		OVI_OP_UNCUT, _plugin[3], OVI_OP_COLON, _effect[3], OVI_OP_AND,
		OVI_OP_UNCUT, _plugin[4], OVI_OP_COLON, _effect[4]
	};
	ASSERT_TRUE(validate_logic(request, _pm.get()));

	LogicAnalyzer logicAnalyzer(request, _pm.get());

	// Even if the result of plugins is 'false', the include is 'true'
	EXPECT_EQ(logicAnalyzer.nextPlugin(true), _plugin[0]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[1]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[2]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[3]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), _plugin[4]);
	EXPECT_EQ(logicAnalyzer.nextPlugin(false), OVI_EOP);
	EXPECT_TRUE(logicAnalyzer.include());
}
