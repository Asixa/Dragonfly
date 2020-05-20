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

		CodeGen::BuildInFunc("malloc", CodeGen::void_ptr,std::vector<llvm::Type*>{CodeGen::int32});

		CodeGen::BuildInFunc("memcpy", CodeGen::void_ptr,
			std::vector<llvm::Type*>{CodeGen::void_ptr, CodeGen::void_ptr, CodeGen::int32});

		CodeGen::BuildInFunc("free", CodeGen::void_type,std::vector<llvm::Type*>{CodeGen::void_ptr});

		CodeGen::BuildInFunc("printf", CodeGen::void_type,std::vector<llvm::Type*>{CodeGen::void_ptr}, true);

        for(auto i=0;i<declarations.size();i++)declarations[i]->GenHeader();
		// for (auto& declaration : declarations)declaration->GenHeader();
		for (auto& declaration : declarations)declaration->Gen();

		const auto __df_global_var_init = llvm::Function::Create(llvm::FunctionType::get(CodeGen::void_type, false), llvm::GlobalValue::ExternalLinkage, "__df_global_var_init", CodeGen::the_module.get());
		CodeGen::builder.SetInsertPoint(CodeGen::CreateBasicBlock(__df_global_var_init, "entry"));
		for (auto i : CodeGen::metadata_init) {
			const auto metadata = CodeGen::GetGenericMetaConstant(std::get<0>(i));
		    CodeGen::builder.CreateStore(llvm::ConstantInt::get(CodeGen::int32, std::get<1>(i)),CodeGen::builder.CreateStructGEP(metadata, 0));
			CodeGen::builder.CreateStore(llvm::ConstantInt::get(CodeGen::int32, std::get<2>(i)), CodeGen::builder.CreateStructGEP(metadata, 1));
		}
		CodeGen::builder.CreateRetVoid();

		const auto main_func = CodeGen::CreateMainFunc();
		const auto entry = CodeGen::CreateBasicBlock(main_func, "entry");
		CodeGen::builder.SetInsertPoint(entry);

        if(!CodeGen::metadata_init.empty()) {
			CodeGen::builder.CreateCall(__df_global_var_init, std::vector<llvm::Value*>{});
        }
		for (auto& statement : statements)if (statement != nullptr)statement->Gen();
		CodeGen::builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), 0));
		verifyFunction(*main_func);
	}
} 
