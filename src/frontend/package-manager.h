#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H
#include "frontend/lexer.h"
#include <string>
#include <vector>

class PackageManager {
public:
    class Package {
    public:
		std::string name;
		std::string path;
		std::vector<std::string> df_files;
		std::vector<std::string> cpp_sources;
		std::vector<std::string> cpp_includes;
		std::vector<std::string> cpp_libraries;
    };

	static std::vector<Package*>packages;
	static std::vector<std::string>imported;
	static void QueryPackages();
	static Package* GetPackage(std::wstring name);
	static void Import(Package* pack);
};
#endif
