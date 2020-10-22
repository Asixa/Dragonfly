#ifndef CLASS_DECL_H
#define CLASS_DECL_H
#include "declaration.h"
#include "AST/expressions/expr.h"
#include "frontend/lexer.h"
#include "function-decl.h"

namespace AST {
	namespace decl {
		// class for matching class declaration.
		class ClassDecl final : public Declaration {
		public:
			enum { kInterface, kClass, kStruct };
			int category = kClass;

			bool is_template = false;
			bool one_line = false;
			std::string full_name;
			std::string name;

			AST::Type base_type_name;
			std::vector<std::string> fields;
			std::vector<AST::Type> types;
			std::vector<AST::Type> interfaces;
			std::vector<std::shared_ptr<Declaration>> declarations;
			std::vector<std::shared_ptr<FunctionDecl>> functions;
			std::vector<std::shared_ptr<FunctionDecl>> constructor;
			std::shared_ptr<GenericParam> generic;
			std::shared_ptr<FunctionDecl> destructor;
			static std::shared_ptr<ClassDecl> Parse(int type = kClass);
			void Gen(std::shared_ptr<DFContext>) override;
			void GenHeader(std::shared_ptr<DFContext>) override;
			void Instantiate(std::shared_ptr<DFContext> context, std::shared_ptr<GenericParam> param);
		};
	}
}
#endif