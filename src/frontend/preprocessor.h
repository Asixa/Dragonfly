#ifndef   PREPROCESSOR_H
#define   PREPROCESSOR_H
#include <vector>
#include <string>
#include "package-manager.h"
namespace frontend {
	class Preprocessor {
		static std::wstring LoadFile(std::string file);
	public:
		static std::wstring code;
		static std::vector<int>file_numbers;
		static std::vector<std::string>files;
		static std::vector<std::wstring>lines;
		static std::wstring MapFileNumber(int& number);
		static void AddFile(std::string f);
		static void Process();
	};
}
#endif
