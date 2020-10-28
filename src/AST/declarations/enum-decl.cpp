#include "AST/declarations/enum-decl.h"
using namespace AST::decl;
void EnumDecl::AnalysisHeader(std::shared_ptr<DFContext>) {}
void EnumDecl::Analysis(std::shared_ptr<DFContext>) {}
void AST::decl::EnumDecl::GenHeader(std::shared_ptr<DFContext> context) {}
void AST::decl::EnumDecl::Gen(std::shared_ptr<DFContext> context) {}

std::shared_ptr<AST::decl::EnumDecl> AST::decl::EnumDecl::Parse() {
	auto instance = std::make_shared<EnumDecl>();

	return instance;
}

std::string EnumDecl::GetName() { return "[Enum not implented]"; }
