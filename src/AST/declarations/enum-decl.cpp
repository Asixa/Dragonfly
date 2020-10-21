#include "AST/declarations/enum-decl.h"
void parser::EnumDecl::GenHeader(std::shared_ptr<DFContext> context) {}
void parser::EnumDecl::Gen(std::shared_ptr<DFContext> context) {}

std::shared_ptr<parser::EnumDecl> parser::EnumDecl::Parse() {
	auto instance = std::make_shared<EnumDecl>();

	return instance;
}
