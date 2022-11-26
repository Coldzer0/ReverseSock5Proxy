#include "ConfigIni.h"

//Pretty straight forward, using the mINI header lib.

namespace ConfigIni {

	const char* CONFIG_FILE_NAME = "C2Config.ini";

	mINI::INIFile file(CONFIG_FILE_NAME);
	mINI::INIStructure ini;

	void Init() {
		file.read(ini);
	}

	void Save() {
		file.generate(ini);
	}

	int GetInt(const std::string& section, const std::string& key, int defaultVal) {
		if (ini.has(section) && ini[section].has(key)) {

			int ret;
			try {
				std::string& configVal = ini[section][key];
				ret = std::stoi(configVal);
			}
			catch (...) {
				ret = defaultVal;
			}
			return ret;
		} else {
			SetInt(section, key, defaultVal);
			return defaultVal;
		}
	}

	std::string GetString(const std::string& section, const std::string& key, std::string defaultVal) {
		if (ini.has(section) && ini[section].has(key)) {
			std::string& configVal = ini[section][key];
			return configVal;
		} else {
			SetString(section, key, defaultVal);
			return defaultVal;
		}
	}

	void SetString(const std::string& section, const std::string& key, std::string& value) {
		ini[section][key] = value;
		Save();
	}

	void SetInt(const std::string& section, const std::string& key, int value) {
		ini[section][key] = std::to_string(value);
		Save();
	}
}


