#include "parser.h"
#include <sstream>
#include "codegen.h"
#include "declarations/enum-decl.h"
#include "declarations/extern-decl.h"

namespace parser {

    void Program::ParseSingle() {
        switch (Lexer::token->type) {
        case NewLine: Lexer::Next(); break;
        case K_func:
        case K_dfunc:
        case K_kernal: declarations.push_back(FunctionDecl::Parse()); break;
        // case K_extern: declarations.push_back(FunctionDecl::Parse(true)); break;
        case K_import: Import::Parse(); break;
		case K_class: declarations.push_back(ClassDecl::Parse(ClassDecl::kClass)); break;
		case K_interface: declarations.push_back(ClassDecl::Parse(ClassDecl::kInterface)); break;
		case K_struct: declarations.push_back(ClassDecl::Parse(ClassDecl::kStruct)); break;
		case K_namespace: declarations.push_back(Namespace::Parse()); break;
		case K_enum:      declarations.push_back(EnumDecl::Parse()); break;
		case K_extern: declarations.push_back(Extern::Parse()); break;
        default: statements.push_back(Statement::Parse());
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
        if (!Debugger::only_tokenize)return parser::Program::Parse();
        while (Lexer::peek > 0 && Lexer::token != nullptr) {
            *Debugger::out << "[" << Lexer::Token::Name(Lexer::token->type) << "] ";
            if (Lexer::Check(NewLine) || Lexer::Check(';'))*Debugger::out << "\n";
            Lexer::Next();
        }
        return nullptr;
    }


	void Program::Gen() {


		CodeGen::BuildInFunc("malloc", CodeGen::void_ptr,std::vector<llvm::Type*>{CodeGen::int32});

		CodeGen::BuildInFunc("memcpy", CodeGen::void_ptr,
			std::vector<llvm::Type*>{CodeGen::void_ptr, CodeGen::void_ptr, CodeGen::int32});

		CodeGen::BuildInFunc("free", CodeGen::void_type,std::vector<llvm::Type*>{CodeGen::void_ptr});

		CodeGen::BuildInFunc("printf", CodeGen::void_type,std::vector<llvm::Type*>{CodeGen::void_ptr}, true);

		for (auto i = 0; i < declarations.size(); i++) 
			try {declarations[i]->GenHeader();}catch (int e) {}
		
		for (auto& declaration : declarations) 
			try { declaration->Gen(); }catch (int e) {}

		// for (auto i = 0; i < late_declarations.size(); i++)
		// 	try { late_declarations[i]->GenHeader(); } catch (int e) {}
        


		const auto __df_global_var_init = llvm::Function::Create(llvm::FunctionType::get(CodeGen::void_type, false), llvm::GlobalValue::ExternalLinkage, "__df_global_var_init", CodeGen::the_module.get());
		CodeGen::builder.SetInsertPoint(CodeGen::CreateBasicBlock(__df_global_var_init, "entry"));
		CodeGen::builder.CreateRetVoid();

		const auto main_func = CodeGen::CreateMainFunc();
		const auto entry = CodeGen::CreateBasicBlock(main_func, "entry");
		CodeGen::builder.SetInsertPoint(entry);

        
		for (auto& statement : statements)
			try {if (statement != nullptr)statement->Gen();}catch (int e){}
		CodeGen::builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), 0));

		for (auto i = 0; i < late_gen.size(); i++)
			try { late_gen[i]->Gen(); }catch (int e) {}


		verifyFunction(*main_func);
	}
}


parser::Type Lexer::MatchType() {
	parser::Type type;
	if (Lexer::IsBasicType()) {
		type.ty = token->type;
		type.str = "";
		Lexer::Next();
	}
	else {
		type.ty = 0;
		type.str = Lexer::string_val;
		Lexer::Match(Id);
		while (Check('.')) {
			Next();
			type.str +="." + Lexer::string_val;
			Lexer::Match(Id);
		}
		if (Check('<')) {
			Next();
			type.str += "<";
			if (Lexer::Check(Id)) {
				Lexer::Next();
				type.str += Lexer::string_val;
				while (Lexer::Check(',')) {
					Lexer::Next();
					Lexer::Match(Id);
					type.str += "," + Lexer::string_val;
				}
			}
			Lexer::Match('>');
			type.str += ">";
		}
	}
	if (Check('[')) {
		Next();
        if(Check(Num)) {
            if(token->value!=K_int) {
				Debugger::Error(L"Value in side a array operator shoule be integer.");
                return type;
            }
			type.array = number_val;
            Next();
        }
        else {
			type.array = -2;
        }
		Match(']');
	}
	return type;
}
