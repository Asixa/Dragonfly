#include "AST/declarations/enum-decl.h"
void parser::EnumDecl::GenHeader() {}
void parser::EnumDecl::Gen() {}

std::shared_ptr<parser::EnumDecl> parser::EnumDecl::Parse() {
	auto instance = std::make_shared<EnumDecl>();

	return instance;
}
