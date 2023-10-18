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

#include <string>
#include <iostream>
#include <map>
#include <cassert>
#include <algorithm>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <getopt.h>
#include <ovi.h>
#include "Log.h"

#define CRESET	"\x1b[0m"
#define CRED	"\x1b[31m"
#define CGREEN	"\x1b[32m"
#define CLYELLOW	"\x1b[93m"	//light yellow
#define CLMAGEN	"\x1b[95m"	//light magenta

#define MAX_STRING_LEN	2048

using namespace ovi;

using StringVec = std::vector<std::string>;

#define OVI_FAILED(ret) ((ret) != OVI_ERROR_NONE)

#define THROW_IF_FAILED(expr, exception) \
do { \
	if (auto ret = (expr); OVI_FAILED(ret)) \
		throw exception; \
} while (0)

struct PluginInfo
{
	std::string name;
	std::map<std::string, std::string>attrs;
};

static std::atomic_bool stopFlag;

class CmdParser
{
public:
	CmdParser(int argc, char **argv);
	~CmdParser() = default;

	std::string _inputPath;
	int _skipVideoFrames {};
	PluginInfo _render;
	std::vector<PluginInfo> _linkedPlugins;
	int _verboseLevel = ovi::logger::LOG_LEVEL_ERROR;

private:
	void parsePlugin(const std::string& linkedPlugins);
	PluginInfo parsePluginInfo(const std::string& pluginStr);

	const char *pluginDelimiter = "()";
	const char *attrDelimiter = "=,";
};

CmdParser::CmdParser(int argc, char **argv)
{
	while (1) {
		int opt;
		int opt_idx = 0;
		static struct option long_options[] = {
			{"skv"		, required_argument,	0, 's'},
			{"version"	, no_argument,			0, 'V'},
			{"help"		, no_argument,			0, 'h'},
			{"verbose"	, required_argument,	0, 'v'},
			{0, 0, 0, 0}
		};

		if ((opt = getopt_long_only(argc, argv, "i:r:l:v:h", long_options, &opt_idx)) == -1)
			break;

		std::cout.width(30);
		std::cout << std::left;
		switch (opt) {
		case 'i':
			std::cout << CGREEN "input file path" CRESET << optarg << std::endl;
			_inputPath = optarg;
			break;

		case 'r':
			std::cout << CGREEN "render info" CRESET << optarg << std::endl;
			_render = parsePluginInfo(optarg);
			break;

		case 's':
			std::cout << CGREEN "skip video option" CRESET << optarg << std::endl;
			_skipVideoFrames = std::stoi(optarg);
			break;

		case 'l':
			std::cout << CGREEN "linked plugins" CRESET << optarg << std::endl;
			parsePlugin(optarg);
			break;

		case 'v':
			std::cout << CGREEN "verbose level" CRESET << optarg << " (trace:0 debug:1 info:2 warn:3 error:4 critical:5 off:6)" << std::endl;
			_verboseLevel = std::stoi(optarg);
			break;

		case 'V':
			std::cout.width(0);
			std::cout << "ovi version " << "0.1" << std::endl; //ToDo
			throw std::invalid_argument("V");

		case 'h':
			throw std::invalid_argument("h");

		case '?':
		default:
			std::cout << CRED "session could not be constructed: syntax error" CRESET << std::endl;
			throw std::invalid_argument("?");
		}
	}
}

void CmdParser::parsePlugin(const std::string& linkedPlugins)
{
	const StringVec pluginList = [&]() {
		const char* pluginDelimiter = " ";
		char *cstr = strdup(linkedPlugins.c_str());
		assert(cstr);

		StringVec vec;

		for (char *plugin = strtok(cstr, pluginDelimiter); plugin; plugin = strtok(nullptr, pluginDelimiter))
			vec.push_back(plugin);

		free(cstr);

		return vec;
	}();

	for (const auto& pluginStr : pluginList)
		_linkedPlugins.push_back(parsePluginInfo(pluginStr));
}

PluginInfo CmdParser::parsePluginInfo(const std::string& pluginStr)
{
	char *cstr = strdup(pluginStr.c_str());

	// Plugin Name
	char *pluginName = strtok(cstr, pluginDelimiter);
	assert(pluginName);
	//std::cout << "pluginName:" << pluginName << std::endl;

	PluginInfo plugin { .name = pluginName };

	// Plugin Attributes
	char *attrs = strtok(nullptr, pluginDelimiter);
	if (!attrs) {
		//std::cout << "No attribute" << std::endl;
		free(cstr);
		return plugin;
	}
	//std::cout << "attrs:" << attrs << std::endl;

	StringVec attrPair;

	for (char* attr = strtok(attrs, attrDelimiter); attr; attr = strtok(nullptr, attrDelimiter))
		attrPair.push_back(attr);

	free(cstr);

	for (size_t i = 0; i < attrPair.size(); i += 2)
		plugin.attrs[attrPair[i]] = attrPair[i + 1];

	return plugin;
}

static void __session_error_cb(void *handle, ovi_error_e error, void *user_data)
{
	LOG_ERROR("__session_error_cb() is invoked, error:%d, user_data:%p", error, user_data);
	stopFlag = true;
}

static void __session_progress_cb(void *handle, const char *progress, void *user_data)
{
	LOG_DEBUG("__session_progress_cb() is invoked, progress:%s, user_data:%p", progress, user_data);
}

static void __session_state_changed_cb(void *handle, ovi_state_e previous, ovi_state_e current, void *user_data)
{
	LOG_DEBUG("__session_state_changed_cb() is invoked, previous:%d, current:%d", previous, current);

	if (previous == OVI_STATE_RENDER && current == OVI_STATE_IDLE)
		stopFlag = true;
}

class SessionManager
{
public:
	explicit SessionManager(const CmdParser& parser);
	~SessionManager();

	void run();

private:
	void setup(const CmdParser& parser);
	void destroy();

	void setRender(PluginInfo plugin);
	StringVec addPlugins(const std::vector<PluginInfo>& linkedPlugins);
	void linkPlugins(const StringVec& pluginsToLink);

	const std::map<std::string, std::string> operatorMap {
		{ "&", OVI_OP_AND },
		{ "|", OVI_OP_OR },
		{ ":", OVI_OP_COLON },
		{ "~", OVI_OP_UNCUT }
	};

	session _session {};
};

SessionManager::SessionManager(const CmdParser& parser)
{
	setup(parser);
}

SessionManager::~SessionManager()
{
	destroy();
}

void SessionManager::setup(const CmdParser& parser)
{
	try {
		/* create */
		THROW_IF_FAILED(ovi_session_create(&_session),
						std::runtime_error("failed to ovi_session_create()"));

		/* callback */
		THROW_IF_FAILED(ovi_session_set_error_cb(_session, __session_error_cb, nullptr),
						std::runtime_error("failed to ovi_session_set_error_cb()"));
		THROW_IF_FAILED(ovi_session_set_progress_cb(_session, __session_progress_cb, nullptr),
						std::runtime_error("failed to ovi_session_set_progress_cb()"));
		THROW_IF_FAILED(ovi_session_set_state_changed_cb(_session, __session_state_changed_cb, nullptr),
						std::runtime_error("failed to ovi_session_set_state_changed_cb()"));

		/* input mediaPath */
		if (parser._inputPath.empty())
			throw std::runtime_error("No input media path");

		THROW_IF_FAILED(ovi_session_set_media_path(_session, parser._inputPath.c_str()),
						std::runtime_error("failed to ovi_session_set_media_path()"));

		/* render */
		setRender(parser._render);

		/* options*/
		if (parser._skipVideoFrames > 0) {
			THROW_IF_FAILED(ovi_session_set_skip_video_frames(_session, parser._skipVideoFrames),
							std::runtime_error("failed to ovi_session_set_skip_video_frames()"));
		}

		/* link plugins */
		if (parser._linkedPlugins.empty())
			throw std::runtime_error("No plugin to run");

		linkPlugins(addPlugins(parser._linkedPlugins));
	} catch (const std::exception& e) {
		destroy();
		throw;
	}
}

void SessionManager::run()
{
	THROW_IF_FAILED(ovi_session_start(_session),
					std::runtime_error("failed to ovi_session_start()"));
}

void SessionManager::setRender(PluginInfo plugin)
{
	std::string outputPath;

	if (plugin.name.empty())
		throw std::runtime_error("No render");

	for (const auto& [key, value] : plugin.attrs) {
		if (key != "path") {
			LOG_ERROR("Unsupported parameter:%s-%s", key.c_str(), value.c_str());
			throw std::runtime_error("Unsupported parameter");
		}

		outputPath = value;
	}

	if (outputPath.empty())
		throw std::runtime_error("No output path of render");

	const char *uid = nullptr;
	THROW_IF_FAILED(ovi_session_add_plugin(_session, plugin.name.c_str(), &uid),
					std::runtime_error("failed to ovi_session_add_plugin()"));

	THROW_IF_FAILED(ovi_session_set_render(_session, uid, outputPath.c_str()),
					std::runtime_error("failed to ovi_session_set_render()"));
}

StringVec SessionManager::addPlugins(const std::vector<PluginInfo>& linkedPlugins)
{
	StringVec itemsTolink;

	for (const auto &plugin : linkedPlugins) {
		auto iter = operatorMap.find(plugin.name);
		if (iter != operatorMap.end()) {
			itemsTolink.push_back(iter->second);
			continue;
		}

		const char *uid = nullptr;
		THROW_IF_FAILED(ovi_session_add_plugin(_session, plugin.name.c_str(), &uid),
						std::runtime_error("failed to ovi_session_add_plugin()"));

		for (const auto& [key, value] : plugin.attrs) {
			LOG_DEBUG("%s:%s", key.c_str(), value.c_str());

			THROW_IF_FAILED(ovi_session_set_plugin_attribute(_session, uid, key.c_str(), value.c_str(), nullptr),
							std::runtime_error("failed to ovi_session_set_plugin_attribute()"));
		}

		itemsTolink.push_back(uid);
	}

	return itemsTolink;
}

void SessionManager::linkPlugins(const StringVec& pluginsToLink)
{
	std::vector<const char *> pluginsToLinkConst;

	std::for_each(pluginsToLink.begin(), pluginsToLink.end(),
		[&](const auto& item) {
			pluginsToLinkConst.push_back(item.c_str());
			LOG_DEBUG("%s", item.c_str());
		}
	);

	THROW_IF_FAILED(ovi_session_link_plugins_with_list(_session,
														pluginsToLinkConst.data(),
														pluginsToLinkConst.size()),
					std::runtime_error("failed to ovi_session_link_plugins_with_list()"));
}

void SessionManager::destroy(void)
{
	if (!_session)
		return;

	ovi_session_destroy(_session);
}

static void showUsage(void)
{
	std::cout << CLYELLOW "Usage:" CRESET << std::endl;
	std::cout << "\tovi_session [Session Launch Options] [Application Options]" << std::endl;
	std::cout << CLYELLOW "\nSession Launch Options:" CRESET << "\n"
		<< "\t-i			Media path to analyze and edit" << CRED " (mandatory)" CRESET << "\n"
		<< "\t-r			Render Plugin and its attrubutes" << CRED " (mandatory)" CRESET << "\n"
		<< "\t-l			Plugins to link" << CRED " (mandatory)" CRESET << "\n"
		<< "\t-skv			Video Frames count to skip analyze" << "\n"
		<< "\t-v, -verbose		Logging level. Default 4. all:0 debug:1 info:2 warn:3 error:4 off:5" << "\n"
		<< CLMAGEN "\n\tPlugin Link Operators:" CRESET << "\n"
		<< "\t\t&		Link plugins with AND. cut the file according to the analysis result" << "\n"
		<< "\t\t|		Link plugins with OR. cut the file according to the analysis result" << "\n"
		<< "\t\t:		Apply effects to the plugin. It must be behind the plugin" << "\n"
		<< "\t\t~		Analyze video without cutting file. It must be used to apply effects only" << "\n"
		<< CLYELLOW "\nApplication Options:" CRESET << "\n"
		<< "\t-version		Version" << "\n"
		<< "\t-h, -help		help" << "\n"
		<< CLYELLOW "\nExample:" CRESET << "\n"
		<< " $ " << CGREEN "ovi_session" CRESET << " -i ./movie.mp4 -r FFMPEGRender'(path=./result.mp4)' -skv 3 -l 'AudioDetect' -verbose 4" << "\n"
		<< " $ " << CGREEN "ovi_session" CRESET << " -i ./movie.mp4 -skv 3 -l 'AudioDetect(dbThreshold=50)' -r OTIORender'(path=./result.otio)'" << "\n"
		<< std::endl;
	std::cout << CLYELLOW "\nAbort:\n" CRESET << "\tPress Enter to abort the session.\n" << std::endl;
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		showUsage();
		return 0;
	}

	try {
		CmdParser parser(argc, argv);
		logger::init(parser._verboseLevel);

		SessionManager sessionMgr(parser);
		sessionMgr.run();
		stopFlag = false;

		std::thread t([]() {
			std::cin.ignore();
			std::cout << CRED "Quit program" CRESET << std::endl;
			stopFlag = true;
		});
		t.detach();

		while (!stopFlag)
			std::this_thread::sleep_for(std::chrono::seconds(1));

		return 0;

	} catch (const std::invalid_argument& e) {
		std::cout << "Invalid argument: " << e.what() << std::endl;
		showUsage();
	} catch (const std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
	}

	return -1;
}
