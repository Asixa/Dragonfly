#include "debug.h"

std::basic_ostream<wchar_t>* debugger::out = nullptr;
bool debugger::is_std_out = false;
bool debugger::error_existed = false;
bool debugger::error_occurred = false;
bool debugger::only_tokenize = false;
bool debugger::skip_line = true;

int debugger::line = 0;
int debugger::ch = 0;
int debugger::chp = 0;
int debugger::tab = 0;
int debugger::log_color = 0;
wchar_t* debugger::end = nullptr;
std::vector<wchar_t*> debugger::lines;

void debugger::SetStream(const bool t) {
    is_std_out = t;
    out = is_std_out ? &std::wcout : new std::wstringstream();
}

void debugger::WriteOutput(const char* file) {
    if (is_std_out)return;
    std::wofstream stream;
    stream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
    stream.open(file);
    stream << dynamic_cast<std::wstringstream*>(out)->str();
    stream.close();
}

void debugger::WriteReadableIr(llvm::Module* module, const char* file, bool print) {
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

void debugger::WriteBitCodeIr(llvm::Module* module, const char* file) {
    std::error_code ec;
    llvm::raw_fd_ostream os(file, ec, llvm::sys::fs::F_None);
    WriteBitcodeToFile(*module, os);
    os.flush();
}

void debugger::SetColor(const int c) {
    static HANDLE handle;
    if (handle == nullptr)handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, c);
}

void debugger::PrintErrorInfo(const std::wstring type, const bool show_location) {
    if (show_location)
        *out << L"[" << line + 1 << L"," << ch << L"]: ";
    SetColor(log_color);
    *out << type << L": ";
    SetColor(kWhite);
}

void debugger::PrintErrorPostfix() {
    auto pt = lines[line];							//find the start pointer of this line.
    std::wstring str;
    while (*pt != L'\n' && pt < end)str += *pt++;		//push all the characters to str

    SetColor(kDarkTeal);
    *out << str << std::endl;						// print error line of code
    SetColor(log_color);

    auto space = chp - 1;							// calculate red arrow location
    auto error_token = ch - chp - 1;
    space = space < 0 ? 0 : space;					// catch when location is negative
    error_token = error_token < 0 ? 0 : error_token;
    *out << std::wstring(space, L' ') << L"¡ü" << std::wstring(error_token, L'`') << std::endl;
    SetColor(kWhite);
}

void debugger::AlertNewline() {
    chp = ch = --lexer::src - lines[--line];
    lines.pop_back();
    skip_line = false;
}

void debugger::AlertNonBreak(const std::wstring info) {
    error_existed = true;
    log_color = kRed;
    PrintErrorInfo(L"error");
    *out << info << std::endl;
    PrintErrorPostfix();
}

void debugger::Alert(const std::wstring info) {
    error_occurred = true;
    AlertNonBreak(info);
}

void debugger::Warn(const std::wstring info) {
    log_color = kYellow;
    PrintErrorInfo(L"warning");
    *out << info << std::endl;
    PrintErrorPostfix();
}
