#ifndef JUST_IN_TIME
#define JUST_IN_TIME
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h" 
#include "llvm/IR/Verifier.h"
#include "parser.h"

using namespace lexer;
using namespace llvm;
using namespace parser;


	static LLVMContext TheContext;
	static IRBuilder<> Builder(TheContext);
	static std::unique_ptr<Module> TheModule;
	static std::map<std::string, Value*> NamedValues;

	Value* LogErrorV(const char* str) {
		printf("%s", str);
		return nullptr;
	}


namespace parser
{
	inline Value* NumberConst::gen()
	{
		return ConstantFP::get(TheContext, APFloat(value));
	}

	inline Value* Boolean::gen()
	{
		return nullptr;
	}
		
	inline Value* String::gen()
	{
		return nullptr;
	}

	inline Value* Factor::gen()
	{
		return nullptr;
	}
		
	inline Value* Unary::gen()
	{
		return nullptr;
	}

	inline Value* Binary::gen()
	{
		auto L = LHS->gen();
		const auto R = RHS->gen();
		if (!L || !R)return nullptr;
		
		switch (op) {
			case '+':
				return Builder.CreateFAdd(L, R, "addtmp");
			case '-':
				return Builder.CreateFSub(L, R, "subtmp");
			case '*':
				return Builder.CreateFMul(L, R, "multmp");
			case '<':
				L = Builder.CreateFCmpULT(L, R, "cmptmp");
				return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext),
					"booltmp");
			default:
				return LogErrorV("invalid binary operator");
		}
	}


	inline Value* Ternary::gen()
	{
		return nullptr;
	}
	inline Function* Function::gen()
	{
		return nullptr;
	}
	
}
#endif
