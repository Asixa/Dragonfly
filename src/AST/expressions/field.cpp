#include "AST/expressions/field.h"
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

namespace parser {
	void Field::ToString() {
		*Debugger::out << name;
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
		Lexer::Next(); VERIFY
			if (Lexer::token->type == '(' || Lexer::token->type == '[')
			{
				std::shared_ptr<Field> field = nullptr;

				while (true) {
					bool br = false;
					switch (Lexer::token->type) {
					case '(': {
						auto child = field;
						field = FuncCall::Parse(name);
						if (!name.empty())name = L"";
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
		if (parent == nullptr)v = CodeGen::FindField(name,cmd);
		else {
            if(CodeGen::GetValuePtrDepth(parent)>1)
			    parent = CodeGen::builder.CreateLoad(parent);
			v = CodeGen::FindMemberField(parent, name);
		}

		if (child != nullptr)
		{
			printf("phhhh %ws\n", name.c_str());
			child->cmd = cmd;
			v = child->GenField(v);
		}
		
		if (parent == nullptr) {
			
			if (v->getType()->getTypeID() == llvm::Type::PointerTyID && cmd == 0)
				return CodeGen::AlignLoad(CodeGen::builder.CreateLoad(v));
		}

		return v;

	}

}
