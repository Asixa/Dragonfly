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
		//////////////////////////////////////////////////////////////////////////////
		/// State 1£¬ Gen the value, and check.
		//////////////////////////////////////////////////////////////////////////////
		if (value == nullptr) {
			CodeGen::builder.CreateRetVoid();
			return;
		}

		auto val = value->Gen();
		if (!val) {
			CodeGen::LogErrorV("Error in return");
            return;
		}

		//////////////////////////////////////////////////////////////////////////////
        /// State 2.a£¬ for generic function
        //////////////////////////////////////////////////////////////////////////////
		auto const function = CodeGen::builder.GetInsertBlock()->getParent();
        if(CodeGen::current_function->generic_return>=0) {
            const auto return_dest = function->getArg(CodeGen::current_function->generic_return);
            const auto metadata = CodeGen::GetGenericMetaArgument(CodeGen::MangleStr(CodeGen::current_function->return_type));
            llvm::Value* size = CodeGen::builder.CreateLoad(CodeGen::builder.CreateStructGEP(metadata, 0), "generic_size");
            const auto mem_cpy = CodeGen::the_module->getFunction("memcpy");
            CodeGen::builder.CreateCall(mem_cpy, std::vector<llvm::Value*>{return_dest, val, size});
        }
		//////////////////////////////////////////////////////////////////////////////
        /// State 2.b£¬ for normal function
        //////////////////////////////////////////////////////////////////////////////
		else {
			auto const expected = function->getReturnType();
			if (CodeGen::GetStructName(expected) != CodeGen::GetStructName(val)) {
				CodeGen::LogErrorV("return type not same");
				return;
			}
			auto const expected_ptr_level = CodeGen::GetPtrDepth(expected);
			auto val_ptr_level = CodeGen::GetPtrDepth(val);
			while (val_ptr_level > expected_ptr_level) {
				val = CodeGen::builder.CreateLoad(val);
				val_ptr_level--;
			}
			CodeGen::builder.CreateRet(val);
		}
	}
}
