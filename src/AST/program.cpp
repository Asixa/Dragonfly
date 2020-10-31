#include "AST//program.h"
#include <sstream>

#include "AST/declarations/enum-decl.h"
#include "AST/declarations/extern-decl.h"
#include "llvm/IR/Verifier.h"
#include "AST/type.h"
namespace AST {
	using namespace decl;
    void Program::ParseSingle() {
        switch (Lexer::token->type) {
        case NewLine: Lexer::Next(); break;
        case K_func:
        case K_dfunc:
        case K_kernal: declarations.push_back(FunctionDecl::Parse()); break;
        // case K_extern: declarations.push_back(FunctionDecl::Parse(true)); break;
        case K_import:      stmt::Import::Parse(); break;
		case K_class:       declarations.push_back(ClassDecl::Parse(ClassDecl::kClass)); break;
		case K_interface:   declarations.push_back(ClassDecl::Parse(ClassDecl::kInterface)); break;
		case K_struct:      declarations.push_back(ClassDecl::Parse(ClassDecl::kStruct)); break;
		case K_namespace:   declarations.push_back(Namespace::Parse()); break;
		case K_enum:        declarations.push_back(EnumDecl::Parse()); break;
		case K_extern:      declarations.push_back(Extern::Parse()); break;
        default: statements.push_back(stmt::Statement::Parse());
        }
    }

    std::shared_ptr<Program> Program::Parse() {
        auto program = std::make_shared<Program>();
		while (Lexer::peek > 0 && Lexer::token != nullptr) {
			try {
			program->ParseSingle();
			}
			catch (int error) {
				while (Debugger::error_occurred) {
					Debugger::error_occurred = false;
					Lexer::MoveLine();
					Lexer::Next();
				}
			}
		}
        return program;
    }

    std::shared_ptr<Program> Parse() {
        Lexer::Next();
        if (!Debugger::only_tokenize)return AST::Program::Parse();
        while (Lexer::peek > 0 && Lexer::token != nullptr) {
            *Debugger::out << "[" << Lexer::Token::Name(Lexer::token->type) << "] ";
            if (Lexer::Check(NewLine) || Lexer::Check(';'))*Debugger::out << "\n";
            Lexer::Next();
        }
        return nullptr;
    }


	void Program::Gen(std::shared_ptr<DFContext> context) {

	

		// context->BuildInFunc("malloc", context->void_ptr,std::vector<llvm::Type*>{context->int32});
		//
		// context->BuildInFunc("memcpy", context->void_ptr,
		// 	std::vector<llvm::Type*>{context->void_ptr, context->void_ptr, context->int32});
		//
		// context->BuildInFunc("free", context->void_type,std::vector<llvm::Type*>{context->void_ptr});
		//
		// context->BuildInFunc("printf", context->void_type,std::vector<llvm::Type*>{context->void_ptr}, true);


        // DO not make it into the form like below, because the array will change.
		for (auto i = 0; i < declarations.size(); i++)
			try { declarations[i]->GenHeader(context); }
		catch (int e) {}
		
		for (auto& declaration : declarations) 
			try { declaration->Gen(context); }catch (int e) {}

		// for (auto i = 0; i < late_declarations.size(); i++)
		// 	try { late_declarations[i]->GenHeader(); } catch (int e) {}
         


		const auto __df_global_var_init = llvm::Function::Create(llvm::FunctionType::get(context->void_type, false), llvm::GlobalValue::ExternalLinkage, "__df_global_var_init", context->module.get());
		context->builder->SetInsertPoint(context->CreateBasicBlock(__df_global_var_init, "entry"));
		context->builder->CreateRetVoid();

		const auto main_func = context->CreateMainFunc();
		const auto entry = context->CreateBasicBlock(main_func, "entry");
		context->builder->SetInsertPoint(entry);

		context->llvm->CreateScope();
		for (auto& statement : statements)
			try {if (statement != nullptr)statement->Gen(context);}catch (int e){}
		context->builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context->context), 0));
		context->llvm->EndScope();

		for (auto i = 0; i < late_gen.size(); i++)
			try { late_gen[i]->Gen(context); }catch (int e) {}


		verifyFunction(*main_func);
	}

    void Program::Analysis(std::shared_ptr<DFContext> context) {
		DFContext::BuildInFunc(context, "malloc", BasicType::Void_Ptr, { BasicType::Int });
		DFContext::BuildInFunc(context, "memcpy", BasicType::Void_Ptr, { BasicType::Void_Ptr, BasicType::Void_Ptr,  BasicType::Int });
		DFContext::BuildInFunc(context, "free", BasicType::Void_Ptr, { BasicType::Void_Ptr });
		DFContext::BuildInFunc(context, "printf", BasicType::Void_Ptr, { BasicType::Void_Ptr }, true);

		for (auto i = 0; i < declarations.size(); i++)
			try { declarations[i]->AnalysisHeader(context);  }
		catch (int e) {}

		for (auto& declaration : declarations)
			try { declaration->Analysis(context);}
		catch (int e) {}

		context->ast->CreateScope();
		for (auto& statement : statements)
			try { if (statement != nullptr)statement->Analysis(context); }
		catch (int e) {}
		context->ast->EndScope();
    }
}

       

