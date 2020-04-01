#ifndef PARSER
#define PARSER

#define TEMPALTE_PARSE(name,func,condition,Type)static UNI(Expr) name(){auto left = func();if(condition){auto op = token->type;Next();return MAKE(Type)(left, func(), op);}return left;}
#define TEMPALTE_PARSE_LOOP(name,func,condition,Type)static UNI(Expr) name(){auto left = func();while(condition){auto op = token->type;Next();left= MAKE(Type)(left, func(), op);}return left;}
#define UNI(a)std::unique_ptr<a>
#define MAKE(a)std::make_unique<a>
#define UNI(a)a*
#define MAKE(a)new a

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
#include <iostream>
#include "lexer.h"
#include <utility>
#include <vector>

using namespace lexer;
using namespace llvm;
namespace parser
{
	static LLVMContext TheContext;
	static IRBuilder<> Builder(TheContext);
	static std::unique_ptr<Module> TheModule;
	static std::map<std::string, Value*> NamedValues;
	
	class AST{};
	class Expr:public AST { public: virtual ~Expr() {} };


	class Factor:public Expr
	{
	public:
		Token* tok;
		Factor(Token* t):tok(t){}
		static UNI(Expr) Parse();

	};


	class Unary:public Expr
	{
	public:
		int op;
		UNI(Expr) expr;
		Unary(UNI(Expr) expr, const int op):op(op),expr(std::move(expr)){}
		static UNI(Expr) Parse()
		{
			switch (token->type)
			{
			case Sub:
				Next();
				return MAKE(Unary)(Parse(), Sub);
			case '!':
				Next();
				return MAKE(Unary)(Parse(), '!');
			default:
				return Factor::Parse();
			}
		}

		
	};
	
	class Binary final :public Expr
	{
	public:
		int op;
		UNI(Expr) LHS;
		UNI(Expr) RHS;
		Binary(UNI(Expr)lhs, UNI(Expr) rhs,int op):op(op), LHS(lhs),RHS(rhs) {}
		
		TEMPALTE_PARSE_LOOP(SubParse4, Unary::Parse, (token->type == Mul || token->type == Div), Binary)
		TEMPALTE_PARSE_LOOP(SubParse3, SubParse4, token->type == Add || token->type == Sub, Binary)
		TEMPALTE_PARSE(SubParse2, SubParse3, token->type == Gt || token->type == Lt || token->type == Ge || token->type == Le, Binary)
		TEMPALTE_PARSE_LOOP(SubParse1, SubParse2, token->type == Eq || token->type == Ne, Binary)
		TEMPALTE_PARSE_LOOP(Parse,SubParse1, token->type == Or || token->type == And,Binary)
	};
	
	inline Expr* Factor::Parse()
	{
		UNI(Expr) factor;
		switch (token->type)
		{
			case '(':
				Next();
				factor = Binary::Parse();
				Match(')');
				return factor;
		}
		factor = MAKE(Factor)(token);
		Next();
		return factor;
	}

	
	/// function definition
	class FuncParam {
	public:
		static UNI(FuncParam) Parse()
		{
			auto param = MAKE(FuncParam)();
			while (token->type != ')') {
				Match(Id);
				Match(Id);
				if (token->type == ',') Match(',');
			}
			return param;
		}
	};
	
	/// Function - This class represents a function definition itself.
	class Function {
		UNI(FuncParam) param;
		std::unique_ptr<Expr> body;
		Token* name;
		
		bool differentiable = false, kernal = false;
	public:
		static UNI(Function) Parse()
		{
			auto function = MAKE(Function)();
			if (token->type == Dfunc)function->differentiable = true;
			else if (token->type == Kernal)function->kernal = true;
			Next();

			function->name = Match(Id);
			
			Match('(');
			function->param = FuncParam::Parse();
			Match(')');

			if (token->type == ':')
			{
				Next();
				Match(Id);
			}
			if (token->type == NewLine)Next();
			printf("[Parsed] Function declaration\n");
			Match('{');
			// statements();
			Match('}');
			printf("[Parsed] Function end\n");
			return function;
		}
	};
	
	/// Let
	class VaribleDecl
	{
		Token* name=nullptr,*type=nullptr;
		Expr* value;
	public:
		static UNI(VaribleDecl) Parse()
		{
			auto let = MAKE(VaribleDecl)();
			Next();
			let->name = Match(Id);
			if(token->type == ':')
			{
				Next();
				let->type= Match(Id);
			}
			Match(Assign);
			let->value = Binary::Parse();
			if (token->type == ';'||token->type == NewLine)Next();

			printf("[Parsed] Let statement\n");
			return let;
		}
	};


	static void Program()
	{
		while (token->type == NewLine)Next();
		switch (token->type)
		{
			case Let:		VaribleDecl::Parse(); break;
			case Func:
			case Dfunc:
			case Kernal:	Function::Parse(); break;
		}
	}

	static void Parse()
	{
		Next();
		while (peek > 0 && token != nullptr) Program();
		// while (peek > 0) {
		// 	printf("[%s] ", Token::name(token->type));
		// 	if (token->type == NewLine || token->type == ';')printf("\n");
		// 	Next();
		// }
	}
};

#endif
