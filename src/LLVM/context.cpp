
#include <codecvt>
#include <iostream>
#include <fstream>
#include <utility>

#include <llvm/Support/raw_ostream.h>
#include "LLVM/context.h"
#include "frontend/debug.h"

#include "AST/program.h"
#include "AST/type.h"

std::vector<std::shared_ptr<DFContext>> DFContext::contexts;


void DFContext::BuildInFunc(std::shared_ptr<DFContext> ctx,std::string name, std::shared_ptr<AST::Type> ret,  std::vector<std::shared_ptr<AST::Type>> args, const bool isVarArg)  {
	auto decl = std::make_shared<AST::decl::FunctionDecl>();
	decl->return_type = ret;
	decl->args = std::make_shared<AST::decl::FieldList>(args);
	decl->args->type = AST::decl::FieldList::Arguments;
	if (isVarArg)decl->args->content.push_back(std::make_shared<AST::decl::FieldDecl>("...", nullptr)); // var
	ctx->ast->AddExternFunc(name,decl);

    std::vector<llvm::Type*> llvm_types;
    for (const auto ty : args) llvm_types.push_back(ty->ToLLVM(ctx));
	auto f=llvm::Function::Create(llvm::FunctionType::get(ret->ToLLVM(ctx), llvm_types, isVarArg), llvm::Function::ExternalLinkage, name,ctx->module.get());
	// f->addAttribute(1, llvm::Attribute::ReadNone);
	// f->addAttribute(2, llvm::Attribute::NoUnwind);
}

llvm::Function* DFContext::BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg) {
	return llvm::Function::Create(llvm::FunctionType::get(ret, types, isVarArg), llvm::Function::ExternalLinkage, name,module.get());
}
llvm::StructType* DFContext::BuildInType(const char* name) {
	return llvm::StructType::create(context, name);
}

void DFContext::WriteReadableIr(llvm::Module* module, const char* file, bool print) {
	std::string ir;
	llvm::raw_string_ostream ir_stream(ir);
	ir_stream << *module;
	ir_stream.flush();
	std::ofstream file_stream;
	file_stream.open(file);
	file_stream << ir;
	file_stream.close();
	if (print)std::cout << ir;
}

void DFContext::WriteBitCodeIr(llvm::Module* module, const char* file) {
	std::error_code ec;
	llvm::raw_fd_ostream os(file, ec, llvm::sys::fs::F_None);
	WriteBitcodeToFile(*module, os);
	os.flush();
}

void DFContext::Write() {
	WriteReadableIr(module.get(), (std::string(module->getModuleIdentifier().c_str())+".txt").c_str(), true);
	WriteBitCodeIr(module.get(), (std::string(module->getModuleIdentifier().c_str()) + ".ll").c_str());
}




DFContext::DFContext(std::shared_ptr<AST::Program> program) {
    if(program==nullptr)return;

    module  =  std::make_unique<llvm::Module>("Program", context);
    builder = std::make_unique<llvm::IRBuilder<>>(context);
    data_layout = std::make_unique < llvm::DataLayout>(module.get());

    //states
	is_sub_block = false;
	block_begin = nullptr;
	block_end = nullptr;
	current_function = nullptr;

	this->program = program;

}

void DFContext::Gen() {
    for (auto context : contexts) {
		if(context->program)context->program->Gen(context);
		if (!Debugger::error_existed) {
			context->Write();
			std::cout << "Context \""<< context->module->getModuleIdentifier().c_str() <<"\" Compiled successfully "<< "\n";
		}
		else std::cout << "Context \"" << context->module->getModuleIdentifier().c_str() << "\" Compilation Failed " << "\n";
    }
}

void DFContext::Analysis() {
	for (const auto& context : contexts) {
		if (context->program)context->program->Analysis(context);
	}
}

