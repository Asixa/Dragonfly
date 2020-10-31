#include "AST/Type.h"
#include "frontend/lexer.h"
#include "LLVM/context.h"
#include "AST/declarations/class-decl.h"

std::shared_ptr<AST::Type>
AST::BasicType::String = std::make_shared<BasicType>(K_string),
AST::BasicType::Int = std::make_shared<BasicType>(K_int),
AST::BasicType::Long = std::make_shared<BasicType>(K_long),
AST::BasicType::Float=std::make_shared<BasicType>(K_float),
AST::BasicType::Double = std::make_shared<BasicType>(K_double),
AST::BasicType::Void = std::make_shared<AST::BasicType>(K_void),
AST::BasicType::Void_Ptr = std::make_shared<AST::BasicType>(K_string),
AST::BasicType::Boolean = std::make_shared<AST::BasicType>(K_bool);

std::shared_ptr<AST::Type> AST::Type::Match() {
	std::shared_ptr<AST::Type> type=nullptr;
	if (frontend::Lexer::IsBasicType()) 
		type = BasicType::Match();
	else if(frontend::Lexer::Check('(')) 
		type = Tuple::Match();
	else type = CustomType::Match();
	if (frontend::Lexer::Check('[')) 
		type= Tensor::Match(type);
	return type;
}

std::shared_ptr<AST::BasicType> AST::BasicType::Match() {
	auto type = std::make_shared<BasicType>(frontend::Lexer::token->type);
	frontend::Lexer::Next(); 
	return type;
}

llvm::Type* AST::BasicType::ToLLVM(std::shared_ptr<DFContext>context) {
	llvm::Type* llvm_type = nullptr;
	switch (detail) {
	    case K_void:
	    case 1:             llvm_type = llvm::Type::getVoidTy(context->context); break;
	    case K_byte:        llvm_type = llvm::Type::getInt8Ty(context->context); break;
	    case K_short:       llvm_type = llvm::Type::getInt16Ty(context->context); break;
	    case K_int:         llvm_type = llvm::Type::getInt32Ty(context->context); break;
	    case K_long:        llvm_type = llvm::Type::getInt64Ty(context->context); break;
	    case K_float:       llvm_type = llvm::Type::getFloatTy(context->context); break;
	    case K_double:      llvm_type = llvm::Type::getDoubleTy(context->context); break;
	    case K_bool:        llvm_type = llvm::Type::getInt1Ty(context->context); break;
	    case K_string:      llvm_type = llvm::Type::getInt8PtrTy(context->context); break;
	}
	return llvm_type;
}

std::string AST::BasicType::ToString() {
    return frontend::Lexer::Token::Name(detail);
}

AST::CustomType::CustomType(const std::shared_ptr<decl::ClassDecl> decl) :decl(decl),str(decl->GetFullname()), Type(Custom) {} 

std::shared_ptr<AST::CustomType> AST::CustomType::Match() {
	auto type = std::make_shared<AST::CustomType>();
	type->str = frontend::Lexer::string_val;
	frontend::Lexer::Match(Id);
	while (frontend::Lexer::Check('.')) {
		frontend::Lexer::Next();
		type->str += "." + frontend::Lexer::string_val;
		frontend::Lexer::Match(Id);
	}
	if (frontend::Lexer::Check('<')) {
		frontend::Lexer::Next();
		type->str += "<";
		if (frontend::Lexer::Check(Id)) {
			frontend::Lexer::Next();
			type->str += frontend::Lexer::string_val;
			while (frontend::Lexer::Check(',')) {
				frontend::Lexer::Next();
				frontend::Lexer::Match(Id);
				type->str += "," + frontend::Lexer::string_val;
			}
		}
		frontend::Lexer::Match('>');
		type->str += ">";
	}
	return type;
}

llvm::Type* AST::CustomType::ToLLVM(std::shared_ptr<DFContext> ctx) {
	llvm::Type* llvm_type = nullptr;
	if (ctx->IsCustomType(str)) {
		const auto ty = ctx->module->getTypeByName(str);
		llvm_type = ctx->types_table[str]->category == AST::decl::ClassDecl::kClass ? ty->getPointerTo() : static_cast<llvm::Type*>(ty);
	}
	else {
		if (decl == nullptr)decl = ctx->types_table[str]->decl;
		
		llvm_type = ctx->module->getTypeByName(decl->GetFullname());
		if (llvm_type == nullptr) {
            frontend::Debugger::ErrorV((std::string("Unknown Type: ") + str + "\n").c_str(), -1, -1);
			return nullptr;
		}
	}
	return llvm_type;
}

std::string AST::CustomType::ToString() { return  str; }

std::shared_ptr<AST::Type> AST::Tuple::Match() {
	auto type = std::make_shared<AST::Tuple>();
	frontend::Lexer::Match('(');
	type->types.push_back(AST::Type::Match());
	if (frontend::Lexer::Check(')'))return type->types[0];
	while (frontend::Lexer::Check(',')) 
		type->types.push_back(AST::Type::Match());
	frontend::Lexer::Match(')');
	return std::static_pointer_cast<AST::Type>(type);
}

llvm::Type* AST::Tuple::ToLLVM(std::shared_ptr<DFContext>) {return nullptr;}

std::string AST::Tuple::ToString() { return "NOT_IMPLMENTED"; }


std::shared_ptr<AST::Tensor> AST::Tensor::Match(std::shared_ptr<AST::Type> base) {
	auto type = std::make_shared<AST::Tensor>(base);

	frontend::Lexer::Match('[');
	if (frontend::Lexer::Check(Num)) {
		if (frontend::Lexer::token->value != K_int) {
			frontend::Debugger::Error(L"Value in side a array operator shoule be integer.");
			return nullptr;
		}
		// type->array = frontend::Lexer::number_val;
		frontend::Lexer::Next();
	}
	else {
		// type->array = -2;
	}
	frontend::Lexer::Match(']');
	return  type;
}

llvm::Type* AST::Tensor::ToLLVM(std::shared_ptr<DFContext>) {
    return nullptr;
}

std::string AST::Tensor::ToString() { return "NOT_IMPLMENTED"; }
llvm::Type* AST::FunctionType::ToLLVM(std::shared_ptr<DFContext>) { return nullptr; }
std::string AST::FunctionType::ToString() { return "NOT_IMPLMENTED"; }
