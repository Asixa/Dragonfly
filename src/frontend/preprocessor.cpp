#include "preprocessor.h"
#include "frontend/lexer.h"
#include <codecvt>
#include <fstream>
#include <sstream>
#include "package-manager.h"
#include <iostream>
using namespace frontend;
std::wstring Preprocessor::code;
std::vector<int>Preprocessor::file_numbers;
std::vector<std::string>Preprocessor::files;
std::vector<std::wstring>Preprocessor::lines;


std::wstring Preprocessor::LoadFile(const std::string file) {
	std::wifstream wif(file);
	if (wif.fail()) Debugger::PrintErrorInfo(L"No such file or directory");
	wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	std::wstringstream wss;
	wss << wif.rdbuf();
	return wss.str();
}

std::wstring Preprocessor::MapFileNumber(int& number) {
	auto  size = file_numbers.size();
    for (auto i=0;i < size;i++) {
		if (file_numbers[i] > number ) {
			number = number - (i > 0 ? file_numbers[i - 1] : 0) + 1;
			return std::wstring(files[i].begin(), files[i].end());
		}
				
    }
}

void Preprocessor::AddFile(std::string f) {
	files.push_back(f);
}


void Preprocessor::Process() {
	int line_number=0;
    for (auto i=0;i<files.size();i++) {
		
		std::wstringstream source(LoadFile(files[i]));
		std::wstring line;
		while (std::getline(source, line)) {
			line_number++;
           
			if(line[0]==L'#') {
				lines.push_back(L"\n");
				code += L"\n";
				size_t pos = 0;
                
				if ((pos = line.find(L' ')) != std::string::npos) {
					auto token = line.substr(0, pos);
					line.erase(0, pos + 1);

				   
                    if(token!=L"#import")continue;
				}
				else continue;
				
				const auto package = PackageManager::GetPackage(line);
				if (package == nullptr) {
					std::cout << "no package named " << Lexer::MangleStr(line) << std::endl;
					return;
				}
				else  PackageManager::Import(package);

			}
			else {
				lines.push_back(line);
				code += line + L"\n";
			}
		}
		file_numbers.push_back(line_number);
    }
	code += L"\n";
	Lexer::size = code.size();
	Lexer::root = Lexer::src =const_cast<wchar_t*>(code.c_str());
	Lexer::end = Lexer::root + Lexer::size;
	Debugger::lines.push_back(Lexer::src);
}

