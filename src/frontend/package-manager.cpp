#include <filesystem>
#include "package-manager.h"
#include <fstream>
#include "json.hpp"
#include "preprocessor.h"
using namespace frontend;

using json = nlohmann::json;

std::vector<PackageManager::Package*>PackageManager::packages;
std::vector<std::string>PackageManager::imported;
void PackageManager::QueryPackages() {
	// printf("querying packages\n");
	std::vector<std::string> r;
	for (auto& p : std::filesystem::recursive_directory_iterator("packages"))
	{
        if((p.path().filename()!="package.json"))continue;
        std::ifstream ifstream;
		ifstream.open(p.path());
		std::string str((std::istreambuf_iterator<char>(ifstream)), std::istreambuf_iterator<char>());
		ifstream.close();
        
		// std::cout << p.path().parent_path().filename() << std::endl;
		// std::cout << str << std::endl;
		auto j = json::parse(str);

	    auto package = new Package();
		package->name = p.path().parent_path().filename().string();
		package->path = p.path().parent_path().string();
		std::string string;

		auto data=j["df_files"];
		for (auto i = 0; i < data.size(); i++) {
			data[0].get_to(string);
			package->df_files.push_back(string);
		}
		data = j["cpp_source"];
		for (auto i = 0; i < data.size(); i++) {
			data[0].get_to(string);
			package->cpp_sources.push_back(string);
		}
		data = j["cpp_includes"];
		for (auto i = 0; i < data.size(); i++) {
			data[0].get_to(string);
			package->cpp_includes.push_back(string);
		}
		data = j["cpp_libraries"];
		for (auto i = 0; i < data.size(); i++) {
			data[0].get_to(string);
			package->cpp_libraries.push_back(string);
		}
		packages.push_back(package);
	}
	
}

PackageManager::Package* PackageManager::GetPackage(std::wstring name) {
    for (auto package : packages) {
		if (package->name == Lexer::MangleStr(name))
			return package;
    }
    return nullptr;
}

void PackageManager::Import(Package* pack) {
    if(std::find(imported.begin(),imported.end(),pack->name)==imported.end()) {
		for (const auto& i : pack->df_files)
			Preprocessor::AddFile(pack->path + "/" + i);
		imported.push_back(pack->name);
    }
}

