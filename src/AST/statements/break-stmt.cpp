#include "AST/statements/break-stmt.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<Break> Break::Parse() {
		Lexer::Next();
		auto instance = std::make_shared<Break>();
		return instance;
	}

	void Break::Gen() {
		if (!CodeGen::is_sub_block) {
			Debugger::ErrorNonBreak(L"invalid_token :break");
			return;
		}
		CodeGen::builder.CreateBr(CodeGen::block_end);
	}
}
