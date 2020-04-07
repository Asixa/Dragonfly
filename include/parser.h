#ifndef PARSER
#define PARSER

#define UNI(a)a*
#define MAKE(a)new a

#define CHECK token->type==
#define CHECK_TYPE CHECK K_int || CHECK K_short || CHECK K_long || CHECK K_float || CHECK K_double ||\
CHECK K_bool || CHECK K_string || CHECK K_uint || CHECK K_ushort || CHECK K_short || CHECK K_byte
#define GEN	void print() override; Value* Gen(const int cmd=0) override;

#include <iostream>
#include "lexer.h"
#include <utility>
#include <vector>
#include <llvm/IR/Value.h>
using namespace lexer;
using namespace llvm;
namespace parser
{
	class AST
	{
		public: virtual Value* Gen(const int cmd=0)=0;
	};
	class Expr :public AST {
	public:
		int type;
		virtual ~Expr() {}
		virtual void print() = 0;
	};
	class Statement
	{
		public: virtual void Gen() = 0;
		static UNI(Statement) Parse();
	};
	class Declaration:public Statement
	{
	public:
		virtual void GenHead() = 0;
	};
	
	class Statements :Statement
	{
		UNI(Statement) stmt1;
		UNI(Statement) stmt2;
	public:
		Statements(UNI(Statement) a, UNI(Statement) b):stmt1(a),stmt2(b){}
		static UNI(Statement) Parse()
		{
			while (CHECK NewLine)Next();
			if(CHECK '}') return nullptr;
			const auto left = Statement::Parse();
			const auto right = Statements::Parse();
			if (right == nullptr)
			{
				return left;
			}
			return MAKE(Statements)(left, right);
		}
		void Gen() override;
	};


	class NumberConst final :public Expr
	{
	public:
		double value;
		explicit NumberConst(const double d, const int t) :value(d) { type = t; }
		GEN
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
		std::wstring value;
		String(std::wstring d) :value(std::move(d)) {}
	};

	class Lambda final :public Expr
	{
		public:GEN
		static UNI(Lambda) Parse(){return nullptr;}
	};
	
	class Field final :public Expr
	{
	public:GEN
		std::vector<std::wstring> names;
		Field(std::vector<std::wstring> d) :names(d) {}
	};
	class FuncCall final :public Expr
	{
	public:GEN
		std::vector <std::wstring> names;
		std::vector<UNI(Expr)> args;
		FuncCall(std::vector <std::wstring> d) : names(d) {}
		static UNI(FuncCall) Parse(std::vector <std::wstring> f);
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
		bool prefix;
		UNI(Expr) expr;
		
		Unary(UNI(Expr) expr, const int op,bool pre):op(op),expr(std::move(expr)),prefix(pre){}
		static UNI(Expr) ParsePrefix()
		{
			switch (token->type)
			{
			case '-':
			case '!':
			case Inc:
			case Dec:
				Next();
				return MAKE(Unary)(Parse(), token->type,true);
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
				return MAKE(Unary)(factor, token->type,false);
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
		// PARSE_LOOP(Sub0, , (CHECK'.')) // Wrong
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
	
	inline FuncCall* FuncCall::Parse(std::vector <std::wstring> f)
	{
		auto func_call = MAKE(FuncCall)(f);
		Next();
		while (token->type != ')') {
			func_call->args.push_back(Binary::Parse());
			if (CHECK ',') Match(',');
		}
		Match(')');
		return func_call;
	}

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
		case '(': {
			const auto point = src;
			auto is_lambda = false;
			Find('(', ')');
			Next();
			printf("ahead token is :%s\n", Token::Name(token->type));
			is_lambda = token->type == Arrow;
				
			printf("isLambda:%d   ", is_lambda);
			src = point;
			Next();
			printf("current token is :%s\n", Token::Name(token->type));
			system("PAUSE");
			if (is_lambda)
			{

			}
			else {
				
				factor = Binary::Parse();
				Match(')');
				return factor;
			}
		}
		case Str:
		{
			auto str = MAKE(String)(string_val);
			Next();
			return str;
		}

		case Num:
		{
			const auto ty = token->value;
			Next();
			return MAKE(NumberConst)(number_val, ty);
		}
		case K_true:
			Next();
			return MAKE(Boolean)(true);
		case K_false:
			Next();
			return MAKE(Boolean)(false);
		case K_int: case K_short: case K_long: case K_float: case K_double:
		case K_uint:case K_ushort:case K_ulong:case K_string:
		case Id:
			// const auto id = string_val;
			std::vector<std::wstring>names;
			names.push_back(string_val);
			Next();
			const auto type = token->type;
			while (token->type=='.')
			{
				Next();
				names.push_back(string_val);
				Match(Id);
			}
			if (CHECK '(')return FuncCall::Parse(names);
			else if (CHECK '[')
			{

			}
			else return MAKE(Field)(names);
		}
		factor = MAKE(Factor)(token);
		Next();
		return factor;
	}



	/// function definition
	class FuncParam {
	public:
		int size;
		bool isVarArg = false;
		std::vector<std::wstring> names;
		std::vector<std::wstring> types;
		static UNI(FuncParam) Parse()
		{
			auto param = MAKE(FuncParam)();
			while (token->type != ')') {
				param->size++;
				if (CHECK_TYPE)
				{
					std::wstring t; t+= static_cast<wchar_t>(token->type) ;
					param->types.push_back(t);
					Next();
				}
				else if(token->type=='.')
				{
					Next();
					Match('.');
					Match('.');
					param->size--;
					param->isVarArg = true;
					return param;
				}
				else
				{
					param->types.push_back(string_val);
					Match(Id);
				}
				
				param->names.push_back(string_val);
				Match(Id);
				if (CHECK ',') Match(',');
			}
			return param;
		}
	};
	
	/// Function - This class represents a function definition itself.
	class FunctionDecl final:public Declaration {
		
		UNI(FuncParam) args;
		std::wstring name;
		UNI(Statement) statements;

		std::wstring return_type;
	
		bool differentiable = false, kernal = false,is_extern=false;
	public:
		void Gen() override;
		void GenHead() override;
		static UNI(FunctionDecl) Parse(bool ext=false)
		{
			auto function = MAKE(FunctionDecl)();
			function->is_extern = ext;
			if (CHECK K_dfunc)function->differentiable = true;
			else if (CHECK K_kernal)function->kernal = true;
			Next();
			if(CHECK Id)
			{
				function->name = string_val;
				Match(Id);
			}
			Match('(');
			function->args = FuncParam::Parse();
			Match(')');

			function->return_type = '0';
			if (CHECK ':')
			{
				Next();
				if(CHECK_TYPE)
				{
					function->return_type = static_cast<wchar_t>(token->type);
					Next();
				}
				else
				{
					function->return_type= string_val;
					Match(Id);
				}
				
			}
			
			if (CHECK NewLine)Next();
			if(ext)
			{
				printf("[Parsed] Extern function declaration\n");
				return function;
			}
			printf("[Parsed] Function declaration\n");
			Match('{');

			function->statements = Statements::Parse();
			while (CHECK NewLine)Next();
			
			Match('}');
			printf("[Parsed] Function end\n");
			return function;
		}
	};
	


	class FieldDecl:public Statement
	{
		bool constant;
		std::wstring name, type;
		Expr* value;
	public:
		static UNI(FieldDecl) Parse(const bool is_const)
		{
			auto let = MAKE(FieldDecl)();
			let->constant = is_const;
			Next();
			let->name = string_val;
			Match(Id);
			if(CHECK ':')
			{
				Next();
				if (CHECK_TYPE)
				{
					let->type = static_cast<wchar_t>(token->type);
					Next();
				}
				else
				{
					let->type = string_val;
					Match(Id);
				}
			}
			Match('=');
			let->value = Binary::Parse();
			
			if (CHECK ';'||CHECK NewLine)Next();

			printf("[Parsed] %s field declaration\n", is_const ?"Constant":"Variable");
			let->value->print();
			printf("\n");
			return let;
		}
		void Gen() override;
		
	};
	
	class StructDecl:public Declaration
	{
		
	public:
		std::wstring name;
		std::vector<std::wstring> fields;
		std::vector<std::wstring> types;
		static UNI(StructDecl) Parse()
		{
			auto instance = MAKE(StructDecl);
			Next();
			instance->name = string_val;
			Match(Id);
			Match('{');
			while (token->type != '}') {
				if (CHECK_TYPE)
				{
					std::wstring t; t += static_cast<wchar_t>(token->type);
					instance->types.push_back(t);
					Next();
				}
				else
				{
					instance->types.push_back(string_val);
					Match(Id);
				}
				instance->fields.push_back(string_val);
				Match(Id);
				if (CHECK ';') Match(';');
				else Match(NewLine);
			}
			Match('}');
			return instance;
		}
		void Gen() override;
		void GenHead() override;
	};


	
	class Else :Statement
	{

	public:
		static UNI(Else) Parse()
		{

		}
		void Gen() override;
	};
	
	class Elif :Else
	{

	public:
		static UNI(Elif) Parse()
		{

		}
		void Gen() override;
	};
	
	class If:Statement
	{
		UNI(Expr) condition;
		UNI(Statement) stmts;
		UNI(Else) else_stmt;
	public:
		static UNI(If) Parse()
		{
			auto instance = new If();
			Next();
			instance->condition = Binary::Parse();
			const auto brackets = CHECK '{';
			if (brackets)Next();
			instance->stmts = Statements::Parse();
			if (brackets)Match('}');
			if (CHECK K_else||CHECK K_elif) 
				instance->else_stmt = Else::Parse();
			return instance;
		}
		// Value* Gen();
		void Gen() override;
	};
	
	class Empty :public Statement
	{
		UNI(Expr) value;
	public:
		static UNI(Empty) Parse()
		{
			auto instance = MAKE(Empty);
			instance->value = Binary::Parse();
			printf("[Parsed] Expression \n");
			instance->value->print();
			return instance;
		}
		void Gen() override;
	};
	
	class Throw:Statement
	{
		UNI(Expr) value;
	public:
		static UNI(Throw) Parse()
		{
			Next();
			auto instance = MAKE(Throw);
			instance->value = Factor::Parse();
			return instance;
		}
		void Gen() override;
	};

	class Return :public Statement
	{
		UNI(Expr) value;
	public:
		static UNI(Return) Parse()
		{
			Next();
			auto instance = MAKE(Return);
			instance->value = Binary::Parse();
			printf("[Parsed] Return Statement\n");
			return instance;
		}
		void Gen() override;
	};

	class Break :Statement
	{
	public:
		static UNI(Break) Parse()
		{
			Next();
			auto instance = MAKE(Break);
			return instance;
		}
		void Gen() override;
	};

	class Continue :Statement
	{
	public:
		static UNI(Continue) Parse()
		{
			Next();
			auto instance = MAKE(Continue);
			return instance;
		}
		void Gen() override;
	};
	
	class Import :Statement
	{
	public:
		static UNI(Import) Parse()
		{
			Next();
			auto instance = MAKE(Import);
			return instance;
		}
		void Gen() override;
	};
	
	inline Statement* Statement::Parse()
	{
		while (CHECK NewLine)Next();
		switch (token->type)
		{
		case K_let:		return FieldDecl::Parse(true);
		case K_var:		return FieldDecl::Parse(false); 
		case K_return:	return Return::Parse();
		default: return Empty::Parse();
		}
	}

	class Program
	{
		std::vector<Statement*> statements;
		std::vector<Declaration*> declarations;
		void ParseSingle() {
			// while (token->type== )Next();
			// if(peek <= 0 || token == nullptr)return;
			
			switch (token->type)
			{
			case NewLine:Next(); break;

			case K_func:
			case K_dfunc:
			case K_kernal:	declarations.push_back(FunctionDecl::Parse());break;
			case K_extern:	declarations.push_back(FunctionDecl::Parse(true));break;
			case K_import:	Import::Parse(); break;
			case K_struct:	declarations.push_back(StructDecl::Parse()); break;
			// case K_let:		statements.push_back(FieldDecl::Parse(true)); break;
			// case K_var:		statements.push_back(FieldDecl::Parse(false)); break;
				// case If:
				// case While:
				// case For
			default: statements.push_back(Statement::Parse());
			}
		}
	public:
		static UNI(Program) Parse()
		{
			auto program = MAKE(Program);
			while (peek > 0 && token != nullptr) program->ParseSingle();
			printf("\n-----------------\n");
			return program;
		}

		void Gen();
		
		
	};

	static UNI(Program) Parse()
	{
		Next();
		return Program::Parse();	
		while (peek > 0) {printf("[%s] ", Token::Name(token->type));	if (CHECK NewLine || CHECK ';')printf("\n");Next();}return nullptr;
	}
};

#endif
