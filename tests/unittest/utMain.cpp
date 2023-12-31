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

#include "Log.h"
#include "utBase.h"

static const std::string LOG_PATH = "log/log_unittest.txt";

int main(int argc, char *argv[])
{
	InitGoogleTest(&argc, argv);

	logger::init(LOG_PATH);
	int ret = RUN_ALL_TESTS();
	logger::reset();

	return ret;
}