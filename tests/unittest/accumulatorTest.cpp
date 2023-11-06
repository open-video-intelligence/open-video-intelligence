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
#include "Accumulator.h"

class AccumulatorTest : public UtBase {
protected:

	Accumulator _accumulator;
};

TEST_F(AccumulatorTest, append_test)
{
	_accumulator.append(9, false);
	_accumulator.append(10, true);
	_accumulator.append(11, true);
	_accumulator.append(12, true);
	_accumulator.append(13, false);

	auto res = _accumulator.accumulated();

	EXPECT_EQ(res.size(), 5);
}

TEST_F(AccumulatorTest, accumulated_test)
{
	_accumulator.append(9, false);
	OVIRect r = {10, 10, 10, 10};
	_accumulator.append(10, true, {{ "plugin.1", { r }}});
	_accumulator.append(11, true);
	_accumulator.append(12, true);
	_accumulator.append(13, false);

	auto res = _accumulator.accumulated();

	EXPECT_EQ(res.size(), 5);
	EXPECT_EQ(res[0].frameNumber, 9);
	EXPECT_EQ(res[0].include, false);

	OVIRect getResult = std::get<OVIRect>(res[1].detected["plugin.1"][0]);
	EXPECT_EQ(getResult.x, 10);
	EXPECT_EQ(getResult.y, 10);
	EXPECT_EQ(getResult.width, 10);
	EXPECT_EQ(getResult.height, 10);
}
