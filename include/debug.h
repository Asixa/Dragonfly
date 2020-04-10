
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
									// Define a output stream.
#ifndef PRINT_TO_STREAM
#define PRINT std::wcout
#else
#define PRINT outstream
#endif

									// this micro should be called each time AST parsed a node, to stop immediately if there are error.
#define VERIFY {if(error_occurred)return nullptr;} 

static bool error_existed;			// Once called ALERT, error_existed will be true and will not be changed, and the compiler won't generate IR.
static bool error_occurred;			// Once called ALERT, error_occurred will be true, and all AST parsing will stop immediately,
									// error_occurred will be false once lexer moved to the next line.
static bool only_tokenize;			// only_tokenize is used for testing lexer, when it is true, compiler will print token buffer, and won't parse AST.
static bool skipline = true;		// to Solve an error, the lexer will skip current line, but for special cases like NEWLINE, lexer will not skip.

static std::vector<wchar_t*>lines;	// collection of pointers to the beginning of each line of source code.
									// line is the number of current line, tab is 1 if there are tabs in this line, otherwise is 0
static int line, ch, chp, tab;		// ch is the right location of error token, chp is the left location of error token. 
static int log_color;				// the color the debugger going to use while print infos.
static wchar_t* end;				// pointer to the last character of  sourcecode

// Write all output to file , for debug and testing.
static void WriteOutput(const char* file)
{
	std::wofstream basic_ofstream;
	basic_ofstream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	basic_ofstream.open(file);
	basic_ofstream << outstream.str();
	basic_ofstream.close();
}
// Write human-readable ir to file , for debug and testing.
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
// Write compilable ir to file , for further compilation.
static void WriteBitCodeIR(llvm::Module* module, const char* file)
{
	std::error_code ec;
	llvm::raw_fd_ostream os(file, ec, llvm::sys::fs::F_None);
	WriteBitcodeToFile(*module, os);
	os.flush();
}

// this special micro will be called when the error token is "Newline", then we need to go back a line to print debug info.
#define ALERT_NEWLINE chp=ch=--src-lines[--line]; lines.pop_back(); skipline=false;

// micro for throw a error.
#define ALERT(a)\
	error_occurred = true;\
	error_existed = true;\
	log_color=Red;\
	PrintErrorInfo(L"error");\
	PRINT<<a << std::endl;\
	PrintErrorPostfix();

// micro for throw a warning.
#define WARN(a)\
	log_color=Yellow;\
	PrintErrorInfo(L"warning");\
	PRINT<<std::wcout << a << std::endl;\
	PrintErrorPostfix();

// enum of console color.
enum Color { Darkblue = 1, Darkgreen, Darkteal, Darkred, Darkpink, Darkyellow, Gray, Darkgray, Blue, Green, Teal, Red, Pink, Yellow, White };

inline void SetColor(const int c);	// this function is implemented in main.cpp,
									// because there are some issues with include<windows.h> before other headers.

//This function will print the current token's location (if show_location=true) and the "error" or "warning"
static void PrintErrorInfo(const std::wstring type, const bool show_location=true) {
		if(show_location)
		PRINT<<L"[" << line+1 << L"," << ch << L"]: ";
		SetColor(log_color);
		PRINT << type<<L": ";
		SetColor(White);
	}

//This function will print the current token and the line of code it belongs.
static void PrintErrorPostfix()
{
	auto pt = lines[line];							//find the start pointer of this line.
	std::wstring str;
	while (*pt != L'\n'&&pt<end)str += *pt++;		//push all the characters to str

	SetColor(Darkteal);
	PRINT<<str << std::endl;						// print error line of code
	SetColor(log_color);
	
	auto space = chp - 1;							// calculate red arrow location
	auto error_token = ch - chp - 1;
	space = space < 0 ?0: space;					// catch when location is negative
	error_token = error_token < 0 ? 0 : error_token;
	PRINT<<std::wstring(space, L' ') << L"↑" << std::wstring(error_token, L'`') << std::endl;
	SetColor(White);
}

#endif

//To avoid the Circular Dependency, The first half of this file will be included first, and the second half will be included after "gen.h"
#ifdef PARSER
#ifndef OUTPUT
#define OUTPUT
#include "parser.h"

// This function takes a type enum like SubAssign, K_float, and return its name "-=" "float"
inline const char* Token::Name(const int type)
{
		if (type == Id)return "Identifier";
		if (type == NewLine) return "NewLine";
		if (type == Num) return "Num";
		if (type == Str) return "Str";
		if (type == 0) return "ERROR";
													// This part using micro to automaticly write converter for Operator and Keywords, see "keywords.h"
#define TOKEN(a)if(type==K_##a)return #a;
		KEYWORDS(TOKEN)
#define TOKEN(a,b)if(type==a)return b;
		OPERATORS(TOKEN)
#undef TOKEN						
		return (new std::string(1, static_cast<char>(type)))->c_str();	// some type is just a chat, like ';' then we just return itself in a string.
}

// implement all the ToString function for Expression nodes below.
inline void Factor::ToString() { PRINT<<"[Factor]"; }
inline void NumberConst::ToString()
{

	switch (type)
	{
	case K_int:PRINT << "["<< static_cast<int>(value)<<"]";return;
	case K_float:PRINT << "["<<value<<"]", value; return;
	case K_double:PRINT << "[" << value << "]"; return;
	default:PRINT << "[" << Token::Name(type) << "]"; return;
	}
}
inline void String::ToString() { PRINT << "[\"" << value << "\"]"; }
inline void Boolean::ToString() { PRINT << "["<< (value ? "true" : "false")<<"]"; }
inline void Field::ToString() { PRINT << "[" <<names[0] << "]"; }
inline void FuncCall::ToString()
{
	PRINT << "[CALL " <<names[0]<<" ( ";
	for (auto i=0;i<args.size();i++)
	{
		args[i]->ToString();
		PRINT << ",";
	}
	PRINT << " )]\n";
}
inline void Unary::ToString() { PRINT << "<"<< Token::Name(op)<<">"; expr->ToString(); }
inline void Binary::ToString() { PRINT << "("; LHS->ToString(); PRINT << " " << Token::Name(op) << " "; RHS->ToString(); PRINT << ")"; }
inline void Ternary::ToString()
{
	PRINT << "[";
	a->ToString();
	PRINT << "?";
	b->ToString();
	PRINT << ":";
	c->ToString();
	PRINT << "]";
}

#endif
#endif
