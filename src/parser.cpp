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
		case K_class: declarations.push_back(ClassDecl::Parse(ClassDecl::kClass)); break;
		case K_interface: declarations.push_back(ClassDecl::Parse(ClassDecl::kInterface)); break;
		case K_struct: declarations.push_back(ClassDecl::Parse(ClassDecl::kStruct)); break;
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

		CodeGen::DeclMetadataStruct();

		
		

		CodeGen::BuildInFunc("malloc", llvm::Type::getInt8PtrTy(CodeGen::the_context),
			std::vector<llvm::Type*>{llvm::Type::getInt32Ty(CodeGen::the_context)});
		CodeGen::BuildInFunc("free", llvm::Type::getVoidTy(CodeGen::the_context),
			std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(CodeGen::the_context)});
		CodeGen::BuildInFunc("printf", llvm::Type::getVoidTy(CodeGen::the_context),
			std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(CodeGen::the_context)}, true);

        for(auto i=0;i<declarations.size();i++)declarations[i]->GenHeader();
		// for (auto& declaration : declarations)declaration->GenHeader();
		for (auto& declaration : declarations)declaration->Gen();

		const auto main_func = CodeGen::CreateMainFunc();
		const auto entry = CodeGen::CreateBasicBlock(main_func, "entry");
		CodeGen::builder.SetInsertPoint(entry);

		// auto c = llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), 233);
		// auto size = CodeGen::builder.CreateConstGEP1_32(a, 0,"233");
		// auto z = CodeGen::builder.CreateStore(size, llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), 233));
		auto a = CodeGen::CreateMetadata("A", 8, 8);
		CodeGen::builder.CreateCall(CodeGen::the_module->getFunction("#metadata()"),std::vector<llvm::Value*>{a});

		for (auto& statement : statements)if (statement != nullptr)statement->Gen();
		CodeGen::builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), 0));
		verifyFunction(*main_func);
	}
} 
