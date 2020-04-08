#ifndef PARSER
#define PARSER

#define UNI(a)a*
#define MAKE(a)new a

//Micro for check token's type
#define CHECK token->type==		
//Micro for check if token is basic type
#define CHECK_TYPE	 CHECK K_int || CHECK K_short || CHECK K_long || CHECK K_float || CHECK K_double ||CHECK K_bool || CHECK K_string || CHECK K_uint || CHECK K_ushort || CHECK K_short || CHECK K_byte
//Micro for overriding the functions for expression classes
#define GEN	void print() override; Value* Gen(const int cmd=0) override;



#include <iostream>
#include "lexer.h"
#include <utility>
#include <vector>
#include <llvm/IR/Value.h>

#define  VERIFY {if(error_occurred)return nullptr;}


using namespace lexer;
using namespace llvm;
namespace parser
{
	//********************************************************************************************************
	//*							AST
	//********************************************************************************************************

	// Base Class for Expressions
	class Expr 
	{
	public:
		virtual ~Expr() {}
		virtual void print() = 0;
		virtual Value* Gen(int cmd = 0) = 0;
	};
	
	// Base Class for Statements
	class Statement
	{
	public:
		virtual ~Statement() = default;
		virtual void Gen() = 0;
		static UNI(Statement) Parse();
	};
	
	// Base Class for Declaration, which will generate codes before Statments
	class Declaration:public Statement
	{
	public:
		virtual void GenHeader() = 0;
	};
	
	// 'Statements' Class will match all statements in a binary-tree structure
	class Statements final :Statement
	{
		UNI(Statement) stmt1;
		UNI(Statement) stmt2;
	public:
		Statements(UNI(Statement) a, UNI(Statement) b):stmt1(a),stmt2(b){}
		static UNI(Statement) Parse();
		void Gen() override;
	};

	//********************   Expressions ***********************
	
	// Expression node for literal numbers.
	class NumberConst final :public Expr
	{
	public:
		int type;
		double value;
		explicit NumberConst(const double d, const int t) :value(d) { type = t; }
		GEN
	};
	
	// Expression node for literal booleans.
	class Boolean final :public Expr
	{
	public:GEN
		bool value;
		explicit Boolean(const bool d) :value(d) {}
	};

	// Expression node for literal strings.
	class String final :public Expr
	{
	public:GEN
		std::wstring value;
		String(std::wstring d) :value(std::move(d)) {}
	};

	// Expression node for Lambda expression.
	class Lambda final :public Expr
	{
		public:GEN
		static UNI(Lambda) Parse(){return nullptr;}
	};
	
	// Expression node for variable or fields.
	class Field final :public Expr
	{
	public:GEN
		std::vector<std::wstring> names;
		explicit Field(std::vector<std::wstring> d) :names(d) {}
	};
	
	// Expression node for function calls.
	class FuncCall final :public Expr
	{
	public:GEN
		std::vector <std::wstring> names;
		std::vector<UNI(Expr)> args;
		explicit FuncCall(std::vector <std::wstring> d) : names(d) {}
		static UNI(FuncCall) Parse(std::vector <std::wstring> f);
	};
	
	// Expression node for All general factors (including all expression nodes above)
	class Factor final :public Expr
	{
	public:GEN
		Token* tok;
		explicit Factor(Token* t):tok(t){}
		static UNI(Expr) Parse();
	};
	
	// Expression node for factors with prefix or postfix)
	class Unary final :public Expr
	{
	public:GEN
		int op;
		bool prefix;
		UNI(Expr) expr;
		Unary(UNI(Expr) expr, const int op,bool pre):op(op),expr(std::move(expr)),prefix(pre){}
		static UNI(Expr) ParsePrefix();
		static UNI(Expr) ParsePostfix();
		static UNI(Expr) Parse()
		{
			return ParsePostfix();
		}
	};

	// Expression node for Ternary expression like a?b:c
	class Ternary final :public Expr
	{
		UNI(Expr) a;
		UNI(Expr) b;
		UNI(Expr) c;
	public:GEN
		Ternary(UNI(Expr) x, UNI(Expr) y, UNI(Expr) z) :a(x), b(y), c(z) {}
		static UNI(Expr) Parse();
	};
	
	// Expression node for all binary expressions
	class Binary final :public Expr
	{
	public:GEN
		int op;
		UNI(Expr) LHS;
		UNI(Expr) RHS;
		Binary(UNI(Expr)lhs, UNI(Expr) rhs,int op):op(op), LHS(lhs),RHS(rhs) {}
#define PARSE_ONCE(name,func,condition)static UNI(Expr) name(){auto left = func();VERIFY if(condition){auto op = token->type;Next();  if(CHECK NewLine) {ALERT_LAST_LINE ALERT("unexpected EndOfLine") return nullptr;}   VERIFY return MAKE(Binary)(left, func(), op);} return left;}
#define PARSE_LOOP(name,func,condition)static UNI(Expr) name(){auto left = func();VERIFY  while(condition){auto op = token->type;Next(); if(CHECK NewLine) {ALERT_LAST_LINE ALERT("unexpected EndOfLine") return nullptr;}   VERIFY left= MAKE(Binary)(left, func(), op);} return left;}
		PARSE_LOOP(Sub1, Unary::Parse, CHECK '*' || CHECK '/' ||CHECK '%')
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

	//**********   End of Expressions ****************


	//************************   Statements  ******************************

	// class for matching function declaration parameters; 
	class FuncParam {
	public:
		int size;
		bool isVarArg = false;
		std::vector<std::wstring> names,types;
		static UNI(FuncParam) Parse();
	};

	// class for matching function definition.
	class FunctionDecl final :public Declaration {
		bool differentiable = false,
			kernal = false,
			is_extern = false;
		std::wstring name;
		std::wstring return_type;
		UNI(FuncParam) args;
		UNI(Statement) statements;
	public:
		void Gen() override;
		void GenHeader() override;
		static UNI(FunctionDecl) Parse(bool ext = false);
	};

	// class for matching variable declaration.
	class FieldDecl :public Statement
	{
		bool constant;
		std::wstring name, type;
		Expr* value;
	public:
		void Gen() override;
		static UNI(FieldDecl) Parse(bool is_const);
	};

	// class for matching class declaration.
	class StructDecl final :public Declaration
	{
	public:
		std::wstring name;
		std::vector<std::wstring> fields;
		std::vector<std::wstring> types;
		static UNI(StructDecl) Parse();
		void Gen() override;
		void GenHeader() override;
	};
	
	// class for if statement.
	class If :Statement
	{
		UNI(Expr) condition;
		UNI(Statement) stmts;
	public:
		static UNI(If) Parse();
		void Gen() override;
	};

	class Empty :public Statement
	{
		UNI(Expr) value;
	public:
		static UNI(Empty) Parse();
		void Gen() override;
	};

	class Throw :Statement
	{
		UNI(Expr) value;
	public:
		static UNI(Throw) Parse();
		void Gen() override;
	};

	class Return :public Statement
	{
		UNI(Expr) value;
	public:
		static UNI(Return) Parse();
		void Gen() override;
	};

	class Break final :Statement
	{
	public:
		static UNI(Break) Parse();
		void Gen() override;
	};

	class Continue final :Statement
	{
	public:
		static UNI(Continue) Parse();
		void Gen() override;
	};

	class Import final :Statement
	{
	public:
		static UNI(Import) Parse();
		void Gen() override;
	};
	
	//********************************************************************************************************
	//*							Parser
	//********************************************************************************************************

	inline Statement* Statements::Parse()
	{
		while (CHECK NewLine) { Next(); VERIFY }
		if (CHECK '}') return nullptr;
		const auto left = Statement::Parse(); VERIFY
		const auto right = Statements::Parse(); VERIFY
		if (right == nullptr)return left;
		return MAKE(Statements)(left, right);
	}
	
	inline FuncCall* FuncCall::Parse(std::vector <std::wstring> f)
	{
		auto func_call = MAKE(FuncCall)(f);
		Next();												VERIFY
		while (token->type != ')') {
			func_call->args.push_back(Binary::Parse());		VERIFY
			if (CHECK ',') Match(',');					VERIFY
		}
		Match(')');										VERIFY
		return func_call;
	}

	inline Expr* Unary::ParsePostfix()
	{
		const auto factor = ParsePrefix();
																VERIFY
		switch (token->type)
		{

		case Inc:
		case Dec:
			Next();												VERIFY
			return MAKE(Unary)(factor, token->type, false);
		default:
			return factor;
		}
	}

	inline Expr* Unary::ParsePrefix()
	{
		switch (token->type)
		{
		case NewLine:
			ALERT_LAST_LINE ALERT("unexpected EndOfLine")
			return nullptr;
		case '-':
		case '!':
		case Inc:
		case Dec: {
			Next();											VERIFY
				const auto parsed = Parse();						VERIFY
				return MAKE(Unary)(parsed, token->type, true);
		}
		default:
			return Factor::Parse();	
		}

	}
	
	inline Expr* Ternary::Parse()
	{
		const auto a = Binary::Sub7();		VERIFY
		if (token->type != '?')return a;
		Next();								VERIFY
		const auto b = Binary::Sub7();		VERIFY
		Match(':');						VERIFY
		const auto c = Binary::Sub7();		VERIFY
		return MAKE(Ternary)(a, b, c);
	}

	inline Expr* Factor::Parse()
	{
		
		UNI(Expr) factor;
		switch (token->type)
		{
		case '(': {
			Next();
			// const auto point = src;
			// auto is_lambda = false;
			// Find('(', ')');
			// Next();												VERIFY
			// printf("ahead token is :%s\n", Token::Name(token->type));
			// is_lambda = token->type == Arrow;
			// 	
			// printf("isLambda:%d   ", is_lambda);
			// src = point;
			// Next();												VERIFY
			// printf("current token is :%s\n", Token::Name(token->type));
			// system("PAUSE");
			// if (is_lambda)
			// {
			//
			// }
			// else {
				
				factor = Binary::Parse();								VERIFY
				Match(')');											VERIFY
				return factor;
			// }
		}
		case Str:
		{
			auto str = MAKE(String)(string_val);
			Next(); VERIFY
			return str;
		}

		case Num:
		{
			const auto ty = token->value;
			Next(); VERIFY
			return MAKE(NumberConst)(number_val, ty);
		}
		case K_true:
			Next(); VERIFY
			return MAKE(Boolean)(true);
		case K_false:
			Next(); VERIFY
			return MAKE(Boolean)(false);
		case K_int: case K_short: case K_long: case K_float: case K_double:
		case K_uint:case K_ushort:case K_ulong:case K_string:
		case Id: {
			// const auto id = string_val;
			std::vector<std::wstring>names;
			names.push_back(string_val);
			Next(); VERIFY
				const auto type = token->type;
			while (token->type == '.')
			{
				Next(); VERIFY
					names.push_back(string_val);
				Match(Id); VERIFY
			}
			if (CHECK '(')return FuncCall::Parse(names);
			else if (CHECK '[')
			{
				VERIFY
			}
			else return MAKE(Field)(names);
		}
		default:
			ALERT("unexpected \""<<Token::Name(token->type)<<"\" ")
			return nullptr;
		}
		factor = MAKE(Factor)(token);
		Next(); VERIFY
		return factor;
	}

	inline Statement* Statement::Parse()
	{
	
		while (CHECK NewLine)Next(); VERIFY
		switch (token->type)
		{
		case K_let:		return FieldDecl::Parse(true);
		case K_var:		return FieldDecl::Parse(false); 
		case K_return:	return Return::Parse();
		default:
			return Empty::Parse();
		}
	}
	
	inline FuncParam* FuncParam::Parse()
	{
		auto param = MAKE(FuncParam)();
		while (token->type != ')') {
			param->size++;
			if (CHECK_TYPE)
			{
				std::wstring t; t += static_cast<wchar_t>(token->type);
				param->types.push_back(t);
				Next();				VERIFY
			}
			else if (token->type == '.')		// Parse three dots '...' for variable argument.
			{
				Next();						VERIFY
				Match('.');				VERIFY
				Match('.');				VERIFY
				param->size--;
				param->isVarArg = true;
				return param;
			}
			else
			{
				param->types.push_back(string_val);
				Match(Id);				VERIFY
			}

			param->names.push_back(string_val);
			Match(Id);					VERIFY
			if (CHECK ',') Match(',');	VERIFY
		}
		return param;
	}
	
	inline FunctionDecl* FunctionDecl::Parse(const bool ext)
	{
		auto function = MAKE(FunctionDecl)();
		function->is_extern = ext;
		if (CHECK K_dfunc)function->differentiable = true;
		else if (CHECK K_kernal)function->kernal = true;
		Next();


		VERIFY
		if (CHECK Id)
		{
			function->name = string_val;
			Match(Id); VERIFY
		}
		Match('('); VERIFY
		function->args = FuncParam::Parse();
		Match(')'); VERIFY

		function->return_type = '0';
		if (CHECK ':')
		{
			Next(); VERIFY
			if (CHECK_TYPE)
			{
				function->return_type = static_cast<wchar_t>(token->type);
				Next(); VERIFY
			}
			else
			{
				function->return_type = string_val;
				Match(Id); VERIFY
			}

		}

		if (CHECK NewLine)Next(); VERIFY
		if (ext)
		{
			printf("[Parsed] Extern function declaration\n");
			return function;
		}
		printf("[Parsed] Function declaration\n");
		Match('{'); VERIFY

		function->statements = Statements::Parse();
		while (CHECK NewLine)Next(); VERIFY

		Match('}'); VERIFY
		printf("[Parsed] Function end\n");
		return function;
	}
	
	inline FieldDecl* FieldDecl::Parse(const bool is_const)
	{
		auto let = MAKE(FieldDecl)();
		let->constant = is_const;
		Next(); VERIFY
		let->name = string_val;
		Match(Id);
		if (CHECK ':')
		{
			Next(); VERIFY
			if (CHECK_TYPE)
			{
				let->type = static_cast<wchar_t>(token->type);
				Next(); VERIFY
			}
			else
			{
				let->type = string_val;
				Match(Id); VERIFY
			}
		}
		Match('='); VERIFY
		let->value = Binary::Parse(); VERIFY

		if (CHECK ';') Next();
		else Match(NewLine);
		VERIFY
		// printf("[Parsed] %s field declaration\n", is_const ? "Constant" : "Variable");let->value->print();printf("\n");
		return let;
	}

	inline StructDecl* StructDecl::Parse()
	{
		auto instance = MAKE(StructDecl);
		Next();											VERIFY
		instance->name = string_val;
		Match(Id);									VERIFY
		if (CHECK NewLine)Next();						VERIFY
		Match('{');									VERIFY
		if (CHECK NewLine)Next();						VERIFY
		while (token->type != '}') {			
			if (CHECK NewLine)Next();					VERIFY
			if (CHECK_TYPE)
			{
				std::wstring t; t += static_cast<wchar_t>(token->type);
				instance->types.push_back(t);
				Next(); VERIFY
			}
			else
			{
				instance->types.push_back(string_val);
				Match(Id); VERIFY
			}
			instance->fields.push_back(string_val);
			Match(Id); VERIFY
			if (CHECK ';') Match(';'); 
			else Match(NewLine);
			VERIFY
		}
		Match('}'); VERIFY
		return instance;
	}
	
	inline If* If::Parse()
	{
		auto instance = new If();
		// Next();
		// instance->condition = Binary::Parse();
		// const auto brackets = CHECK '{';
		// if (brackets)Next();
		// instance->stmts = Statements::Parse();
		// if (brackets)Match('}');
		// if (CHECK K_else || CHECK K_elif)
		// 	instance->else_stmt = nullptr;// Else::Parse();
		return instance;
	}
	
	inline Throw* Throw::Parse()
	{
		Next();
		auto instance = MAKE(Throw);
		instance->value = Factor::Parse();
		return instance;
	}
	
	inline Empty* Empty::Parse()
	{
	
		auto instance = MAKE(Empty);
		instance->value = Binary::Parse(); VERIFY
		if (token->type == ';')Next();
		else Match(NewLine);
								VERIFY
		// printf("[Parsed] Expression \n");instance->value->print();
		return instance;
	}

	inline Return* Return::Parse()
	{
		Next();
		auto instance = MAKE(Return);
		instance->value = Binary::Parse();
		printf("[Parsed] Return Statement\n");
		return instance;
	}

	inline Import* Import::Parse()
	{
		Next();
		auto instance = MAKE(Import);
		return instance;
	}
	
	inline Break* Break::Parse()
	{
		Next();
		auto instance = MAKE(Break);
		return instance;
	}
	
	inline Continue* Continue::Parse()
	{
		Next();
		auto instance = MAKE(Continue);
		return instance;
	}



	//***************** Driver ***************
	// Program is a class that will match and store all the codes, it is the root of AST
	class Program
	{
		std::vector<Statement*> statements;
		std::vector<Declaration*> declarations;
		void ParseSingle() {
			switch (token->type)
			{
			case NewLine:Next(); break;

			case K_func:
			case K_dfunc:
			case K_kernal:	declarations.push_back(FunctionDecl::Parse());break;
			case K_extern:	declarations.push_back(FunctionDecl::Parse(true));break;
			case K_import:	Import::Parse(); break;
			case K_struct:	declarations.push_back(StructDecl::Parse()); break;
			default: statements.push_back(Statement::Parse());
			}
			while (error_occurred)
			{
				error_occurred = false;
				MoveLine();
				Next();
				// std::wcout << "<" << *src << ">";
			}
		}
	public:
		static UNI(Program) Parse()
		{
			auto program = MAKE(Program);
			while (peek > 0 && token != nullptr) program->ParseSingle();
			return program;
		}
		void Gen();
	};

	static UNI(Program) Parse()
	{
		Next();
		return Program::Parse();	
		while (peek > 0 && token!=nullptr) {printf("[%s] ", Token::Name(token->type));	if (CHECK NewLine || CHECK ';')printf("\n");Next();}return nullptr;
	}
};



#endif
