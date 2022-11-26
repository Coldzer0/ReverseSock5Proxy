#pragma once

#define MINI_CASE_SENSITIVE
#include "ini.h"

namespace ConfigIni {

	extern const char* CONFIG_FILE_NAME;

	extern mINI::INIFile file;
	extern mINI::INIStructure ini;
	
	int GetInt(const std::string& section, const std::string& key, int defaultVal);
	std::string GetString(const std::string& section, const std::string& key, const std::string& defaultVal);

	void SetString(const std::string& section, const std::string& key, std::string &value);
	void SetInt(const std::string& section, const std::string& key, int value);

	void Init();

	void Save();
}