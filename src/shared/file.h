#pragma once

#include <vector>

namespace yib {
	class File {
	public:
		static std::vector<char> Read(const char* path);
		static bool Write(const char* path, std::vector<char> data);
	};
}