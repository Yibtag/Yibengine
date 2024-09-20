#include "file.h"

#include <fstream>

namespace yib {
	std::vector<char> File::Read(const char* path) {
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			return {};
		}

		uint32_t size = file.tellg();
		std::vector<char> buffer(size);

		file.seekg(0);
		file.read(buffer.data(), size);
		file.close();

		return buffer;
	}

	bool File::Write(const char* path, std::vector<char> data) {
		std::ofstream file(path, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			return false;
		}

		file.seekp(0);
		file.write(data.data(), data.size());
		file.close();

		return true;
	}
}