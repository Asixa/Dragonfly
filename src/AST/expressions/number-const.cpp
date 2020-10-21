#include "AST/expressions/number-const.h"
#include "codegen.h"

namespace parser {
	void NumberConst::ToString() {
		switch (type) {
		case K_int: *Debugger::out << "[" << static_cast<int>(value) << "]";
			return;
		case K_float: *Debugger::out << "[" << value << "]", value;
			return;
		case K_double: *Debugger::out << "[" << value << "]";
			return;
		default: *Debugger::out << "[" << Lexer::Token::Name(type) << "]";
			return;
		}
	}
	llvm::Value* NumberConst::Gen(std::shared_ptr<DFContext> ctx,int cmd) {
		switch (type) {
		case K_float: return llvm::ConstantFP::get(ctx->context, llvm::APFloat(static_cast<float>(value)));
		case K_double: return llvm::ConstantFP::get(ctx->context, llvm::APFloat(value));
		case K_int: return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx->context), static_cast<int>(value));
		default: return Debugger::ErrorV("Unknown number type",line,ch);
		}
	}
}
