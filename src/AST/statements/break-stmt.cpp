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
			Debugger::AlertNonBreak(L"invalid break");
			return;
		}
		CodeGen::builder.CreateBr(CodeGen::block_end);
	}
}
