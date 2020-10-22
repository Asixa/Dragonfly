#include "AST/statements/continue-stmt.h"


namespace AST {
	using namespace  stmt;
	std::shared_ptr<Continue> Continue::Parse() {
        frontend::Lexer::Next();
		auto instance = std::make_shared<Continue>();
		return instance;
	}
	void Continue::Gen(std::shared_ptr<DFContext> ctx) {
		if (!ctx->is_sub_block) {
            frontend::Debugger::ErrorNonBreak(L"invalid_token :continue");
			return;
		}
		ctx->builder->CreateBr(ctx->block_begin);
	}

}
