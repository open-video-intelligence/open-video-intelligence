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
#include "OutcomeCache.h"

class OutcomeCacheTest : public UtBase {
protected:

	OutcomeCache _outcomeCache;
	const std::vector<std::tuple<std::string, Outcome>> TEST_ARGS = {
		{ "test1", {true, {}}},
		{ "test2", {false, {}}},
		{ "test3", {true, {}}},
		{ "test4", {false, {}}},
		{ "test5", {true, {}}},
	};

	const std::string INVALID_UID = "test100";
};

TEST_F(OutcomeCacheTest, hit_check_return_value)
{
	for (const auto& [ uid, outcome ] : TEST_ARGS)
		_outcomeCache.write(uid, outcome);

	for (const auto& [ uid, outcome ] : TEST_ARGS)
		EXPECT_TRUE(_outcomeCache.hit(uid));

	EXPECT_FALSE(_outcomeCache.hit(INVALID_UID));
}

TEST_F(OutcomeCacheTest, write_check)
{
	for (const auto& [ uid, outcome ] : TEST_ARGS) {
		_outcomeCache.write(uid, outcome);
		EXPECT_EQ(_outcomeCache.result().detect, outcome.detect);
	}
}

TEST_F(OutcomeCacheTest, result_check_return_value)
{
	// if empty, result is true
	EXPECT_TRUE(_outcomeCache.result().detect);

	for (const auto& [ uid, outcome ] : TEST_ARGS)
		_outcomeCache.write(uid, outcome);

	for (const auto& [ uid, outcome ] : TEST_ARGS) {
		_outcomeCache.setResultUid(uid);
		EXPECT_EQ(_outcomeCache.result().detect, outcome.detect);
	}
}

TEST_F(OutcomeCacheTest, detected_check_return_value)
{
	for (const auto& [ uid, outcome ] : TEST_ARGS) {
		_outcomeCache.write(uid, outcome);
		_outcomeCache.setDetected(uid);
		DetectedData detected = _outcomeCache.detected();
		EXPECT_TRUE(detected.find(uid) != detected.end());
	}
}