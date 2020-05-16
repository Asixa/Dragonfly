#include "AST/statements/continue-stmt.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<Continue> Continue::Parse() {
		Lexer::Next();
		auto instance = std::make_shared<Continue>();
		return instance;
	}
	void Continue::Gen() {
		if (!CodeGen::is_sub_block) {
			Debugger::AlertNonBreak(L"invalid continue");
			return;
		}
		CodeGen::builder.CreateBr(CodeGen::block_begin);
	}

}
