#include "AST/expressions/field.h"
#include "frontend/lexer.h"
#include "AST/program.h"


namespace AST {
	using namespace expr;
	void AST::Field::ToString() {
		*Debugger::out << name.c_str();
		if (child != nullptr) {
			*Debugger::out << ".";
			child->ToString();
		}
	}

	std::shared_ptr<Field> Field::Parse() {
		auto field = ParsePostfix();
		auto root = field;
		while (Lexer::token->type == '.') {
			Lexer::Next();
			field->child = ParsePostfix();
			field = field->child;
		}
		return root;
	}


	std::shared_ptr<Field> Field::ParsePostfix() {
		auto name = Lexer::string_val;
		Lexer::Next(); 
			if (Lexer::Check('(') || Lexer::Check('<') || Lexer::Check('['))
			{
				std::shared_ptr<Field> field = nullptr;

				while (true) {
					bool br = false;
					switch (Lexer::token->type) {
					case '<': //TODO conflict with less than
					case '(': {
                        const auto child = field;
						field = FuncCall::Parse(name);
						if (!name.empty())name = "";
						field->left = child;
						break;
					}
					case '[': {
						// array access
						break;
					}
					default:
						br = true;
						break;
					}
					if (br)break;
				}
				return field;
			}
			else  return std::make_shared<Field>(name);
	}


	// c is 0 by default, when c is 1, then it keep the pointer of the value.
	llvm::Value* Field::Gen(std::shared_ptr<DFContext> context, const bool is_ptr) {
		this->is_ptr = is_ptr;
		return GenField(context,nullptr);
	}

	std::shared_ptr<AST::Type> Field::Analysis(std::shared_ptr<DFContext>ctx) { return AnalysisField(ctx, nullptr); }
    std::shared_ptr<AST::Type> Field::AnalysisField(std::shared_ptr<DFContext>ctx, std::shared_ptr<AST::Type> parent) {

		std::shared_ptr<AST::Type> v;
        if(!parent) {

			v = ctx->ast->GetField(name);
            if(!v) {
				Debugger::ErrorV(line, ch,"Unknown Variable : {}",name );
				return nullptr;
            }
        }
        else {
			
			v = ctx->ast->GetMemberField(parent,name);
			if (!v) {
				Debugger::ErrorV(line, ch,"cannot get field from variable: {}", parent->ToString() + "." + name );
				return nullptr;
			}
        }

		if (child != nullptr)
		{
			child->is_ptr = is_ptr;
			v = child->AnalysisField(ctx, v);
		}

		return v;
	}

    llvm::Value* Field::GenField(std::shared_ptr<DFContext> ctx,llvm::Value* parent) {

		llvm::Value* v = nullptr;
		// if the parent is null, then we find this field in the scope.
		if (parent == nullptr)v = ctx->llvm->GetField(name,is_ptr);
		// if the parent is not, then we find this field in the parent.
		else {
			// we find parent's first-level-pointer.
			// TODO load once is not safe, should loop untill it is first-level-pointer.
			if (ctx->llvm->GetPtrDepth(parent) > 1)
				parent = ctx->builder->CreateLoad(parent);

			v = ctx->llvm->GetMemberField(parent, name);
		}
		
		// if this field is not done yet. finish it.
		if (child != nullptr) {
			child->is_ptr = is_ptr;
			v = child->GenField(ctx, v);
		}

  //       If we want the constant, when load the pointer.
		// This only done in entry node where parent is null.
		if (parent == nullptr) {
			auto type = ctx->llvm->GetStructName(v->getType());
			if (is_ptr) {  }
			else if (v->getType()->getTypeID() != llvm::Type::PointerTyID) {  }
			else if (child == nullptr && (name == "this" || name == "base")) {  }
			else if (type == "void*") { }
            else if(ctx->ast->GetClass(type) && ctx->ast->GetClassDecl(type)->category== decl::ClassDecl::kClass) {
                 if(!is_ptr) {
                     while (ctx->llvm->GetPtrDepth(v)>1) 
						 v = ctx->llvm->AlignLoad(ctx->builder->CreateLoad(v));
                 }
            }
            else {
                return ctx->llvm->AlignLoad(ctx->builder->CreateLoad(v));
            }
        }
		return v;
	}

}
