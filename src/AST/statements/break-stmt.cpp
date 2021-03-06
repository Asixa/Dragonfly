#include "AST/statements/break-stmt.h"


namespace AST {
	using namespace  stmt;
	std::shared_ptr<Break> Break::Parse() {
        frontend::Lexer::Next();
		auto instance = std::make_shared<Break>();
		return instance;
	}

    void Break::Analysis(std::shared_ptr<DFContext>) {}

    void Break::Gen(std::shared_ptr<DFContext> ctx) {
		if (!ctx->is_sub_block) {
            frontend::Debugger::ErrorNonBreak("invalid_token :break");
			return;
		}
		ctx->builder->CreateBr(ctx->block_end);
	}
}
