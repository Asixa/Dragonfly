#include "AST/expressions/field.h"
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

namespace parser {
	void Field::ToString() {
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
		auto name = CodeGen::MangleStr(Lexer::string_val);
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
	llvm::Value* Field::Gen(const int c) {
		cmd = c;
		return GenField(nullptr);
	}

	llvm::Value* Field::GenField(llvm::Value* parent) {

		llvm::Value* v = nullptr;
		// if the parent is null, then we find this field in the scope.
		if (parent == nullptr)v = CodeGen::FindField(name, cmd);
		// if the parent is not, then we find this field in the parent.
		else {
			// we find parent's first-level-pointer.
			// TODO load once is not safe, should loop untill it is first-level-pointer.
			if (CodeGen::GetPtrDepth(parent) > 1)
				parent = CodeGen::builder.CreateLoad(parent);
			v = CodeGen::FindMemberField(parent, name);
		}

		// if this field is not done yet. finish it.
		if (child != nullptr)
		{
			child->cmd = cmd;
			v = child->GenField(v);
		}

		// If we want the constant, when load the pointer.
		// This only done in entry node where parent is null.
		if (parent == nullptr) {

	
			if (cmd == kConstantWanted && v->getType()->getTypeID() == llvm::Type::PointerTyID
				&& !(child == nullptr && (name == "this" || name == "base"))
				// && (CodeGen::current_function==nullptr||!CodeGen::current_function->IsGenericArgument(name))
				&& !(CodeGen::GetStructName(v->getType())=="void*")
				)
				// TODO load once is not safe, should loop untill it is constant.
				return CodeGen::AlignLoad(CodeGen::builder.CreateLoad(v));
		}
         
		return v;

	}

}
