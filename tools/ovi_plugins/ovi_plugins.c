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


#include <stdio.h>
#include <getopt.h>
#include <ovi.h>


#define CRESET		"\x1b[0m"
#define CRED		"\x1b[31m"
#define CGREEN		"\x1b[32m"
#define CYELLOW		"\x1b[33m"
#define CBLUE		"\x1b[34m"
#define CMAGEN		"\x1b[35m"
#define CCYAN		"\x1b[36m"
#define CLYELLOW	"\x1b[93m"	//light yellow
#define CLBLUE		"\x1b[94m"	//light blue
#define CLMAGEN		"\x1b[95m"	//light magenta
#define CLCYAN		"\x1b[96m"	//light cyan

#define PRINT_RED(fmt, ...) printf(CRED""fmt""CRESET"\n", ##__VA_ARGS__);
#define PRINT_LYELLOW(fmt, ...) printf(CLYELLOW""fmt""CRESET"\n", ##__VA_ARGS__);
#define PRINT_LBLUE(fmt, ...) printf(CLBLUE""fmt""CRESET"\n", ##__VA_ARGS__);
#define PRINT_LMAGEN(fmt, ...) printf(CLMAGEN""fmt""CRESET"\n", ##__VA_ARGS__);
#define PRINT_LCYAN(fmt, ...) printf(CLCYAN""fmt""CRESET"\n", ##__VA_ARGS__);

#define PRINT_PAIR_LBUE(key, value) printf(CLBLUE"%-20s"CRESET"%s\n", key, value);
#define PRINT_PAIR_LBUE_GREEN(key, value) printf(CLBLUE"%-20s"CGREEN"%s\n"CRESET, key, value);


static char *__plugin_type[] = {
	[OVI_PLUGIN_TYPE_NONE] = "OVI_PLUGIN_TYPE_NONE",
	[OVI_PLUGIN_TYPE_VIDEO_DETECT] = "OVI_PLUGIN_TYPE_VIDEO_DETECT",
	[OVI_PLUGIN_TYPE_VIDEO_EFFECT] = "OVI_PLUGIN_TYPE_VIDEO_EFFECT",
	[OVI_PLUGIN_TYPE_AUDIO_DETECT] = "OVI_PLUGIN_TYPE_AUDIO_DETECT",
	[OVI_PLUGIN_TYPE_AUDIO_EFFECT] = "OVI_PLUGIN_TYPE_AUDIO_EFFECT",
	[OVI_PLUGIN_TYPE_RENDER] = "OVI_PLUGIN_TYPE_RENDER",
};

bool __plugin_attribute_foreach_cb(const char *key, const char *type, const char *description, void *user_data)
{
	PRINT_PAIR_LBUE_GREEN("  Name", key);
	PRINT_PAIR_LBUE("  Type", type);
	PRINT_PAIR_LBUE("  Desciption", description);
	printf("\n");

	return true;
}

bool __plugin_foreach_cb(const char *name, ovi_plugin_type_e type, const char *description, void *user_data)
{
	PRINT_PAIR_LBUE_GREEN("  Name", name);
	PRINT_PAIR_LBUE("  Type", __plugin_type[type]);
	PRINT_PAIR_LBUE("  Desciption", description);

	plugin_attribute_foreach_cb cb = (plugin_attribute_foreach_cb)user_data;

	if (cb) {
		PRINT_LYELLOW("\nAttributes:");
		ovi_available_plugin_foreach_attribute(name, cb, NULL);
	}
	printf("\n");

	return true;
}

void foreach_plugin_all()
{
	int i;

	PRINT_LYELLOW("Plugin Details:");

	for (i = 1; i <= OVI_PLUGIN_TYPE_RENDER; i++) {
		PRINT_LMAGEN("  %s:", __plugin_type[i]);
		int ret = ovi_available_plugin_foreach(i, NULL, __plugin_foreach_cb, NULL);
		if (ret != OVI_ERROR_NONE)
			PRINT_RED("failed to get plugin list");
	}
}

void foreach_plugin(ovi_plugin_type_e type, const char *name, void *user_data)
{
	PRINT_LYELLOW("Plugin Details:");

	int ret = ovi_available_plugin_foreach(type, name, __plugin_foreach_cb, user_data);
	if (ret != OVI_ERROR_NONE)
		PRINT_RED("failed to get plugin list");
}

void show_usage(void)
{
	PRINT_LYELLOW("Usage:");
	printf("\tovi_plugins [Options] [Plugin Name]\n");
	PRINT_LYELLOW("\nOptions:");
	printf("\t--list-all				show all available plugins\n");
	printf("\t--list-videodetectors			show all available video detectos\n");
	printf("\t--list-audiodetectors			show all available audio detectos\n");
	printf("\t--list-videoeffects			show all available video ettects\n");
	printf("\t--list-audioeffects			show all available audio ettects\n");
	printf("\t--list-renders				show all available renders\n");
	printf("\t-v, --version				version\n");
	printf("\t-h, --help				help\n");
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		show_usage();
		return 0;
	}

	while (1) {
		int opt;
		int opt_idx = 0;

		static struct option long_options[] = {
			{"help"					, no_argument, 0, 'h'},
			{"version"				, no_argument, 0, 'v'},
			{"list-all"				, no_argument, 0, 'a'},
			{"list-videodetectors"	, no_argument, 0, '1'},
			{"list-audiodetectors"	, no_argument, 0, '2'},
			{"list-videoeffects"	, no_argument, 0, '3'},
			{"list-audioeffects"	, no_argument, 0, '4'},
			{"list-renders"			, no_argument, 0, 'r'},
			{ 0, 0, 0, 0 }
		};

		if ((opt = getopt_long(argc, argv, "hv", long_options, &opt_idx)) == -1)
			break;

		switch (opt) {
		case 'a':
			foreach_plugin_all();
			break;

		case '1':
			foreach_plugin(OVI_PLUGIN_TYPE_VIDEO_DETECT, NULL, NULL);
			break;

		case '2':
			foreach_plugin(OVI_PLUGIN_TYPE_AUDIO_DETECT, NULL, NULL);
			break;

		case '3':
			foreach_plugin(OVI_PLUGIN_TYPE_VIDEO_EFFECT, NULL, NULL);
			break;

		case '4':
			foreach_plugin(OVI_PLUGIN_TYPE_AUDIO_EFFECT, NULL, NULL);
			break;

		case 'r':
			foreach_plugin(OVI_PLUGIN_TYPE_RENDER, NULL, NULL);
			break;

		case 'v':
			printf("ovi plugins version %s\n", "0.1"); //ToDo
			break;

		case 'h':
		default:
			show_usage();
			return 0;
		}

		return 0;
	}

	foreach_plugin(OVI_PLUGIN_TYPE_NONE, argv[1], __plugin_attribute_foreach_cb);

	return 0;
}
