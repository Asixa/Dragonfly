#ifndef PARSER
#define PARSER

#define UNI(a)a*
#define MAKE(a)new a

#define CHECK token->type==
#define GEN	void print() override; Value* gen() override;

#include <iostream>
#include "lexer.h"
#include <utility>
#include <llvm/IR/Value.h>
using namespace lexer;
using namespace llvm;
namespace parser
{
	
	class AST
	{
		public: virtual Value* gen()=0;
	};
	class Expr :public AST {
	public:
		virtual ~Expr() {}
		virtual void print() = 0;
	};

	class NumberConst final :public Expr
	{
	public:
		double value;
		explicit NumberConst(const double d):value(d){}
		Value* gen() override;
		void print() override;
	};

	class Boolean final :public Expr
	{
	public:GEN
		bool value;
		explicit Boolean(const bool d) :value(d) {}
	};
	class String final :public Expr
	{
	public:GEN
		bool value;
		String(char* d) :value(d) {}
	};
	class Factor final :public Expr
	{
	public:GEN
		Token* tok;
		Factor(Token* t):tok(t){}
		static UNI(Expr) Parse();
	};

	class Unary:public Expr
	{
	public:GEN
		int op;
		UNI(Expr) expr;
		
		Unary(UNI(Expr) expr, const int op):op(op),expr(std::move(expr)){}
		static UNI(Expr) ParsePrefix()
		{
			switch (token->type)
			{
			case '-':
			case '!':
				Next();
				return MAKE(Unary)(Parse(), token->type);
			default:
				return Factor::Parse();
			}
		}
		static UNI(Expr) ParsePostfix()
		{
			const auto factor = ParsePrefix();
			
			switch (token->type)
			{
			case Inc:
			case Dec:
				Next();
				return MAKE(Unary)(factor, token->type);
			default:
				return factor;
			}
		}
		static UNI(Expr) Parse()
		{
			return ParsePostfix();
		}

	};
	
	class Ternary final :public Expr
	{
		UNI(Expr) a;
		UNI(Expr) b;
		UNI(Expr) c;
	public:GEN
		Ternary(UNI(Expr) x, UNI(Expr) y, UNI(Expr) z) :a(x), b(y), c(z) {}
		static UNI(Expr) Parse();
	};

	
	class Binary final :public Expr
	{
	public:GEN
		int op;
		UNI(Expr) LHS;
		UNI(Expr) RHS;
		Binary(UNI(Expr)lhs, UNI(Expr) rhs,int op):op(op), LHS(lhs),RHS(rhs) {}
#define PARSE_ONCE(name,func,condition)static UNI(Expr) name(){auto left = func();if(condition){auto op = token->type;Next();return MAKE(Binary)(left, func(), op);}return left;}
#define PARSE_LOOP(name,func,condition)static UNI(Expr) name(){auto left = func();while(condition){auto op = token->type;Next();left= MAKE(Binary)(left, func(), op);}return left;}
		PARSE_LOOP(Sub0, Unary::Parse, (CHECK'.')) // Wrong
		PARSE_LOOP(Sub1, Sub0, CHECK '*' || CHECK '/' ||CHECK '%')
		PARSE_LOOP(Sub2, Sub1, CHECK '+' || CHECK '-')
		PARSE_LOOP(Sub3, Sub2, CHECK Shl || CHECK Shr)
		PARSE_ONCE(Sub4, Sub3, CHECK '>' || CHECK '<' || CHECK Ge || CHECK Le)
		PARSE_LOOP(Sub5, Sub4, CHECK Eq || CHECK Ne)
		PARSE_LOOP(Sub6, Sub5, CHECK '|' || CHECK '^'|| CHECK '&')
		PARSE_LOOP(Sub7,Sub6, CHECK Or || CHECK And)
		PARSE_LOOP(Parse, Ternary::Parse, CHECK '='|| 
			CHECK AddAgn || CHECK SubAgn || CHECK DivAgn || CHECK MulAgn  ||CHECK ModAgn ||  
			CHECK ShlAgn|| CHECK ShrAgn || CHECK BAndAgn||CHECK BXORAgn|| CHECK BORAgn)
	};


	inline Expr* Ternary::Parse()
	{
		const auto a = Binary::Sub7();
		if (token->type != '?')return a;
		Next();
		const auto b = Binary::Sub7();
		Match(':');
		const auto c = Binary::Sub7();
		return MAKE(Ternary)(a, b, c);
	}

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
				if (CHECK ',') Match(',');
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
		Function* gen();
		static UNI(Function) Parse()
		{
			auto function = MAKE(Function)();
			if (CHECK K_dfunc)function->differentiable = true;
			else if (CHECK K_kernal)function->kernal = true;
			Next();

			function->name = Match(Id);
			
			Match('(');
			function->param = FuncParam::Parse();
			Match(')');

			if (CHECK ':')
			{
				Next();
				Match(Id);
			}
			if (CHECK NewLine)Next();
			printf("[Parsed] Function declaration\n");
			Match('{');
			

			// statements();
			while (CHECK NewLine)Next();
			
			Match('}');
			printf("[Parsed] Function end\n");
			return function;
		}
	};
	
	/// Let
	class FieldDecl
	{
		bool constant;
		Token* name=nullptr,*type=nullptr;
		Expr* value;
	public:
		static UNI(FieldDecl) Parse(const bool is_const)
		{
			auto let = MAKE(FieldDecl)();
			let->constant = is_const;
			Next();
			let->name = Match(Id);
			if(CHECK ':')
			{
				Next();
				let->type= Match(Id);
			}
			Match('=');
			let->value = Binary::Parse();
			
			if (CHECK ';'||CHECK NewLine)Next();

			printf("[Parsed] %s field declaration\n", is_const ?"Constant":"Variable");
			let->value->print();
			printf("\n");
			return let;
		}
	};


	static void Program()
	{
		while (CHECK NewLine)Next();
		switch (token->type)
		{
			case K_let:		FieldDecl::Parse(true); break;
			case K_var:		FieldDecl::Parse(false); break;
			case K_func:
			case K_dfunc:
			case K_kernal:	Function::Parse(); break;
			// case Return:
			// case If:
			// case While:
			// case For
		}
	}

	static void Parse()
	{
		Next();
		// while (peek > 0 && token != nullptr) Program();
		while (peek > 0) {printf("[%s] ", Token::Name(token->type));	if (CHECK NewLine || CHECK ';')printf("\n");Next();}
	}
};

#endif
