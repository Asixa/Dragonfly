#include "parser.h"
#include <sstream>
#include "codegen.h"

namespace parser {

    void Program::ParseSingle() {
        switch (Lexer::token->type) {
        case NewLine: Lexer::Next(); break;
        case K_func:
        case K_dfunc:
        case K_kernal: declarations.push_back(FunctionDecl::Parse()); break;
        case K_extern: declarations.push_back(FunctionDecl::Parse(true)); break;
        case K_import: Import::Parse(); break;
		case K_class: declarations.push_back(ClassDecl::Parse()); break;
		case K_interface: declarations.push_back(ClassDecl::Parse(true)); break;
		case K_extension: declarations.push_back(Extension::Parse()); break;
        default: statements.push_back(Statement::Parse());
        }
        while (Debugger::error_occurred) {
            Debugger::error_occurred = false;
            Lexer::MoveLine();
            Lexer::Next();
        }
    }

    std::shared_ptr<Program> Program::Parse() {
        auto program = std::make_shared<Program>();
		while (Lexer::peek > 0 && Lexer::token != nullptr) {
	
			program->ParseSingle();
		}
        return program;
    }

    std::shared_ptr<Program> Parse() {
        Lexer::Next();
        if (!Debugger::only_tokenize)return parser::Program::Parse();
        while (Lexer::peek > 0 && Lexer::token != nullptr) {
            *Debugger::out << "[" << Lexer::Token::Name(Lexer::token->type) << "] ";
            if (Lexer::Check(NewLine) || Lexer::Check(';'))*Debugger::out << "\n";
            Lexer::Next();
        }
        return nullptr;
    }


	void Program::Gen() {
		CodeGen::BuildInFunc("malloc", llvm::Type::getInt8PtrTy(CodeGen::the_context),
			std::vector<llvm::Type*>{llvm::Type::getInt32Ty(CodeGen::the_context)});
		CodeGen::BuildInFunc("free", llvm::Type::getVoidTy(CodeGen::the_context),
			std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(CodeGen::the_context)});
		CodeGen::BuildInFunc("printf", llvm::Type::getVoidTy(CodeGen::the_context),
			std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(CodeGen::the_context)}, true);

		for (auto& declaration : declarations)declaration->GenHeader();
		for (auto& declaration : declarations)declaration->Gen();

		const auto main_func = CodeGen::CreateFunc(CodeGen::builder, "main");
		const auto entry = CodeGen::CreateBb(main_func, "entry");
		// CodeGen::the_function = main_func;
		CodeGen::builder.SetInsertPoint(entry);

		for (auto& statement : statements)if (statement != nullptr)statement->Gen();

		CodeGen::builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), 0));
		verifyFunction(*main_func);
	}
} 
