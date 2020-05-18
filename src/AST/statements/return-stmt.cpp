#include "AST/statements/return-stmt.h"
#include "lexer.h"
#include "AST/expressions/binary.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<Return> Return::Parse() {
		Lexer::Next();
		auto instance = std::make_shared<Return>();
		if (Lexer::token->type == ';' || Lexer::token->type == NewLine) {
			instance->value = nullptr;
			Lexer::Next();
			return instance;
		}
		instance->value = Binary::Parse();
		Lexer::MatchSemicolon();
		*Debugger::out << "[Parsed] Return Statement\n";
		return instance;
	}
	void Return::Gen() {
		if (value == nullptr) {
			CodeGen::builder.CreateRetVoid();
			return;
		}
		auto val = value->Gen();
		if (!val) {
			CodeGen::LogErrorV("Error in return");
            return;
		}
        auto function=CodeGen::builder.GetInsertBlock()->getParent();
		auto expected = function->getReturnType();
        if(CodeGen::GetStructName(expected)!=CodeGen::GetStructName(val)) {
			CodeGen::LogErrorV("return type not same");
            return;
        }
		auto expected_ptr_level = CodeGen::GetPtrDepth(expected);
        auto val_ptr_level= CodeGen::GetPtrDepth(val);
	    while (val_ptr_level>expected_ptr_level) {
			val = CodeGen::builder.CreateLoad(val);
			val_ptr_level--;
        }
		CodeGen::builder.CreateRet(val);
	}
}
