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
						printf("[Parsed FuncCall]\n");
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
		printf("[Analysis field]");
        if(!parent) {
			auto v = ctx->ast->GetField(name);
			printf("[Analysis] Field %s:%s\n", name.c_str(), v->ToString().c_str());
			return v;
        }
		
		return nullptr;
	}

    llvm::Value* Field::GenField(std::shared_ptr<DFContext> ctx,llvm::Value* parent) {

		llvm::Value* v = nullptr;
		// if the parent is null, then we find this field in the scope.
		if (parent == nullptr)v = ctx->llvm->GetField(name,is_ptr);
		// if the parent is not, then we find this field in the parent.
		else {
			// we find parent's first-level-pointer.
			// TODO load once is not safe, should loop untill it is first-level-pointer.
			if (ctx->GetPtrDepth(parent) > 1)
				parent = ctx->builder->CreateLoad(parent);

			v = ctx->llvm->GetMemberField(parent, name);
		}
		printf("child %d %s  \n      > %s<<<<<<<<<<<<<\n", child != nullptr, name.c_str(), ctx->DebugValue(v).c_str());
		// if this field is not done yet. finish it.
		if (child != nullptr) {
			child->is_ptr = is_ptr;
			v = child->GenField(ctx, v);
		}
			
        if(is_ptr||ctx->GetStructName(v->getType())=="void*"||name=="this"){ return v; }
        v=ctx->AlignLoad(ctx->builder->CreateLoad(v));
        

        // If we want the constant, when load the pointer.
		// This only done in entry node where parent is null.
		// if (parent == nullptr) {
		// 	if (cmd == kConstantWanted && v->getType()->getTypeID() == llvm::Type::PointerTyID
		// 		&& !(child == nullptr && (name == "this" || name == "base"))
		// 		// && (ctx->current_function==nullptr||!ctx->current_function->IsGenericArgument(name))
		// 		&& !(ctx->GetStructName(v->getType())=="void*")
		// 		)
		// 		// TODO load once is not safe, should loop untill it is constant.
		// 		return ctx->AlignLoad(ctx->builder->CreateLoad(v));
		// }
         
		

	}

}
