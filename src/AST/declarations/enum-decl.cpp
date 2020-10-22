#include "AST/declarations/enum-decl.h"
void AST::EnumDecl::GenHeader(std::shared_ptr<DFContext> context) {}
void AST::EnumDecl::Gen(std::shared_ptr<DFContext> context) {}

std::shared_ptr<AST::EnumDecl> AST::EnumDecl::Parse() {
	auto instance = std::make_shared<EnumDecl>();

	return instance;
}
