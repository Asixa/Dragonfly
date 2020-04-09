
#ifndef DEBUGGER
#define DEBUGGER
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <codecvt>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/BitcodeWriter.h>

static std::wstringstream outstream;
#ifndef PRINT_TO_STREAM
#define PRINT std::wcout
#else
#define PRINT outstream
#endif
static bool error_existed;
static bool error_occurred;
static bool only_tokenlize;
static void WriteOutput(const char* file)
{
	std::wofstream basic_ofstream;
	basic_ofstream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	basic_ofstream.open(file);
	basic_ofstream << outstream.str();
	basic_ofstream.close();
}

static void WriteReadableIR(llvm::Module* module,const char* file,bool print=false)
{
	std::string ir;
	llvm::raw_string_ostream ir_stream(ir);
	ir_stream << *module;
	ir_stream.flush();
	std::ofstream file_stream;
	file_stream.open(file);
	file_stream << ir;
	file_stream.close();

	if (print)std::cout << ir;
}

static void WriteBitCodeIR(llvm::Module* module, const char* file)
{
	std::error_code ec;
	llvm::raw_fd_ostream os(file, ec, llvm::sys::fs::F_None);
	WriteBitcodeToFile(*module, os);
	os.flush();
}



#define ALERT_LAST_LINE chp=ch=--src-lines[--line]; lines.pop_back(); skipline=false; 
	
#define ALERT(a)\
	error_occurred = true;\
	error_existed = true;\
	log_color=Red;\
	PrintErrorInfo(L"error");\
	PRINT<<a << std::endl;\
	PrintErrorPostfix();

#define WARN(a)\
	log_color=Yellow;\
	PrintErrorInfo(L"warning");\
	PRINT<<std::wcout << a << std::endl;\
	PrintErrorPostfix();

static int line, ch,chp,tab;
static bool skipline=true;
static std::vector<wchar_t*>lines;
static int log_color;

enum Color { Darkblue = 1, Darkgreen, Darkteal, Darkred, Darkpink, Darkyellow, Gray, Darkgray, Blue, Green, Teal, Red, Pink, Yellow, White };

inline void SetColor(const int c);
namespace lexer { static wchar_t* end; static void MoveLine(); }


static void PrintErrorInfo(const std::wstring type, const bool showPosition=true) {
		if(showPosition)
		PRINT<<L"[" << line+1 << L"," << ch << L"]: ";
		SetColor(log_color);
		PRINT << type<<L": ";
		SetColor(White);
	}
static void PrintErrorPostfix()
{
	
	auto pt = lines[line];
	std:std::wstring str;
	while (*pt != L'\n'&&pt<lexer::end)str += *pt++;

	
	SetColor(Darkteal);
	PRINT<<str << std::endl;
	SetColor(log_color);
	auto space = chp - 1;
	auto error_token = ch - chp - 1;


	space = space < 0 ?0: space;
	error_token = error_token < 0 ? 0 : error_token;
	PRINT<<std::wstring(space, L' ') << L"↑" << std::wstring(error_token, L'`') << std::endl;
	SetColor(White);
}

#endif


#ifdef PARSER
#ifndef OUTPUT
#define OUTPUT
#include "parser.h"

inline const char* Token::Name(const int type)
{
		if (type == Id)return "Identifier";
		if (type == NewLine) return "NewLine";
		if (type == Num) return "Num";
		if (type == Str) return "Str";
		if (type == 0) return "ERROR";
#define TOKEN(a)if(type==K_##a)return #a;
		KEYWORDS(TOKEN)
#define TOKEN(a,b)if(type==a)return b;
			OPERATORS(TOKEN)
#undef TOKEN
		return (new std::string(1, static_cast<char>(type)))->c_str();
}


inline void Factor::output() { PRINT<<"[Factor]"; }
inline void NumberConst::output()
{

	switch (type)
	{
	case K_int:PRINT << "["<< static_cast<int>(value)<<"]";return;
	case K_float:PRINT << "["<<value<<"]", value; return;
	case K_double:PRINT << "[" << value << "]"; return;
	default:PRINT << "[" << Token::Name(type) << "]"; return;
	}
	
}
inline void String::output() { PRINT << "[\"" << value << "\"]"; }
inline void Boolean::output() { PRINT << "["<< (value ? "true" : "false")<<"]"; }
inline void Field::output() { PRINT << "[" <<names[0] << "]"; }
inline void FuncCall::output()
{
	
	PRINT << "[CALL " <<names[0]<<" ( ";
	for (auto i=0;i<args.size();i++)
	{
		args[i]->output();
		PRINT << ",";
	}
	PRINT << " )]\n";
}

inline void Unary::output() { PRINT << "<"<< Token::Name(op)<<">"; expr->output(); }
inline void Binary::output() { PRINT << "("; LHS->output(); PRINT << " " << Token::Name(op) << " "; RHS->output(); PRINT << ")"; }
inline void Ternary::output()
{
	PRINT << "[";
	a->output();
	PRINT << "?";
	b->output();
	PRINT << ":";
	c->output();
	PRINT << "]";
}


#endif
#endif
