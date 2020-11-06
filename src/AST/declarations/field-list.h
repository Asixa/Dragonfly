#ifndef FIELD_LIST_DECL_H
#define FIELD_LIST_DECL_H
#include <memory>
#include <string>
#include <vector>
#include "AST/Type.h"

namespace AST {
	namespace decl {
		class FieldDecl {
		public:
			std::string name;
			std::shared_ptr<AST::Type> type;
			int generic;
			FieldDecl(std::string name, std::shared_ptr<AST::Type> type) :name(name), type(type), generic(false) {}
			FieldDecl(std::shared_ptr<FieldDecl>copy) :name(copy->name), type(copy->type), generic(copy->generic) {}

		};

		class FieldList {
		public:
            enum ListType{ GenericDecl, GenericInstantiate, Arguments};
			ListType type;
			// bool is_var_arg;
			std::vector< std::shared_ptr<FieldDecl>>content;
			std::string ToString();
            FieldList(){}
			explicit FieldList(std::shared_ptr<FieldList> copy);
			explicit FieldList(std::vector<std::shared_ptr<AST::Type>>& type);
			static std::shared_ptr<FieldList> ParseGenericDecl();
			static std::shared_ptr<FieldList> ParseGenericInstantiate();
			static std::shared_ptr<FieldList> ParseArguments(const bool parse_var_arg, const bool parse_expr) ;

			int FindByName(std::string name);
			int FindByType(std::string type);
			bool IsVariableArgument();
		};

	}
}
#endif
