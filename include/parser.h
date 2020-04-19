#ifndef PARSER
#define PARSER


#include <iostream>
#include "lexer.h"
#include <utility>
#include <vector>
#include <llvm/IR/DerivedTypes.h>
using namespace lexer;
using namespace llvm;
using namespace debugger;
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
		virtual void ToString() = 0;
		virtual Value* Gen(int cmd = 0) = 0;
	};
	
	// Base Class for Statements
	class Statement
	{
	public:
		virtual ~Statement() = default;
		virtual void Gen() = 0;
		static std::shared_ptr<Statement> Parse();
	};
	
	// Base Class for Declaration, which will generate codes before Statments
	class Declaration:public Statement
	{
	public:
		virtual void GenHeader() = 0;
	};
	
	// 'Statements' Class will match all statements in a binary-tree structure
	class Statements final :public Statement
	{
		std::shared_ptr<Statement> stmt1;
		std::shared_ptr<Statement> stmt2;
	public:
		Statements(std::shared_ptr<Statement> a, std::shared_ptr<Statement> b):stmt1(a),stmt2(b){}
		static std::shared_ptr<Statement> Parse();
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
		void ToString() override;
	    Value* Gen(const int cmd=0) override;
	};
	
	// Expression node for literal booleans.
	class Boolean final :public Expr
	{
	public:
	    void ToString() override;
	    Value* Gen(const int cmd=0) override;
		bool value;
		explicit Boolean(const bool d) :value(d) {}
	};

	// Expression node for literal strings.
	class String final :public Expr
	{
	public:
	    void ToString() override;
	    Value* Gen(const int cmd=0) override;
		std::wstring value;
		String(std::wstring d) :value(std::move(d)) {}
	};

	// Expression node for Lambda expression.
	class Lambda final :public Expr
	{
		public:
	    void ToString() override;
	    Value* Gen(const int cmd=0) override;
		static std::shared_ptr<Lambda> Parse(){return nullptr;}
	};
	
	// Expression node for variable or fields.
	class Field final :public Expr
	{
	public:
	    void ToString() override;
	    Value* Gen(const int cmd=0) override;
		std::vector<std::wstring> names;
		explicit Field(std::vector<std::wstring> d) :names(d) {}
	};
	
	// Expression node for function calls.
	class FuncCall final :public Expr
	{
	public:
	    void ToString() override;
	    Value* Gen(const int cmd=0) override;
		std::vector <std::wstring> names;
		std::vector<std::shared_ptr<Expr>> args;
		explicit FuncCall(std::vector <std::wstring> d) : names(d) {}
		static std::shared_ptr<FuncCall> Parse(std::vector <std::wstring> f);
	};
	
	// Expression node for All general factors (including all expression nodes above)
	class Factor final :public Expr
	{
	public:
	    void ToString() override;
	    Value* Gen(const int cmd=0) override;
		Token* tok;
		explicit Factor(Token* t):tok(t){}
		static std::shared_ptr<Expr> Parse();
	};
	
	// Expression node for factors with prefix or postfix)
	class Unary final :public Expr
	{
	public:
	    void ToString() override;
	    Value* Gen(const int cmd=0) override;
		int op;
		bool prefix;
		std::shared_ptr<Expr> expr;
		Unary(std::shared_ptr<Expr> expr, const int op,bool pre):op(op),expr(std::move(expr)),prefix(pre){}
		static std::shared_ptr<Expr> ParsePrefix();
		static std::shared_ptr<Expr> ParsePostfix();
		static std::shared_ptr<Expr> Parse()
		{
			return ParsePostfix();
		}
	};

	// Expression node for Ternary expression like a?b:c
	class Ternary final :public Expr
	{
		std::shared_ptr<Expr> a;
		std::shared_ptr<Expr> b;
		std::shared_ptr<Expr> c;
	public:
	    void ToString() override;
	    Value* Gen(const int cmd=0) override;
		Ternary(std::shared_ptr<Expr> x, std::shared_ptr<Expr> y, std::shared_ptr<Expr> z) :a(x), b(y), c(z) {}
		static std::shared_ptr<Expr> Parse();
	};
	
	// Expression node for all binary expressions
	class Binary final :public Expr
	{
	public:
	    void ToString() override;
	    Value* Gen(const int cmd=0) override;
		int op;
		std::shared_ptr<Expr> LHS;
		std::shared_ptr<Expr> RHS;
		Binary(std::shared_ptr<Expr>lhs, std::shared_ptr<Expr> rhs,int op):op(op), LHS(lhs),RHS(rhs) {}
#define PARSE_ONCE(name,func,condition)static std::shared_ptr<Expr> name(){auto left = func();VERIFY if(condition){auto op = token->type;Next();  if(lexer::Check( NewLine)) {AlertNewline(); Alert(L"unexpected EndOfLine"); return nullptr;}   VERIFY return std::make_shared<Binary>(left, func(), op);} return left;}
#define PARSE_LOOP(name,func,condition)static std::shared_ptr<Expr> name(){auto left = func();VERIFY  while(condition){auto op = token->type;Next(); if(lexer::Check(NewLine)) {AlertNewline(); Alert(L"unexpected EndOfLine"); return nullptr;}   VERIFY left= std::make_shared<Binary>(left, func(), op);} return left;}
		PARSE_LOOP(Sub1, Unary::Parse, Check('*') || Check( '/') ||Check( '%'))
		PARSE_LOOP(Sub2, Sub1, Check( '+') || Check( '-'))
		PARSE_LOOP(Sub3, Sub2, Check( Shl) || Check( Shr))
		PARSE_ONCE(Sub4, Sub3, Check( '>') || Check( '<') || Check( Ge) || Check( Le))
		PARSE_LOOP(Sub5, Sub4, Check( Eq) || Check( Ne))
		PARSE_LOOP(Sub6, Sub5, Check( '|') || Check( '^') || Check( '&'))
		PARSE_LOOP(Sub7,Sub6, Check( Or) || Check( And))
		PARSE_LOOP(Parse, Ternary::Parse, Check( '=') || lexer::Check({ AddAgn,SubAgn ,SubAgn,DivAgn,MulAgn,ModAgn,ShlAgn,ShrAgn,BAndAgn,BXORAgn,BORAgn }))
	};

	//**********   End of Expressions ****************


	//************************   Statements  ******************************

	// class for matching function declaration parameters; 
	class FuncParam {
	public:
		int size;
		bool isVarArg = false;
		std::vector<std::wstring> names,types;
		static std::shared_ptr<FuncParam> Parse();
	};

	// class for matching function definition.
	class FunctionDecl final :public Declaration {
	public:
		bool differentiable = false,
			kernal = false,
			is_extern = false;
		std::wstring name;
		StructType* self_type;
		std::wstring return_type;
		std::shared_ptr<FuncParam> args;
		std::shared_ptr<Statement> statements;

		void SetInternal(const std::wstring structname,StructType* type)
		{
			name = structname + L"." + name;
			self_type = type;
		}
		void Gen() override;
		void GenHeader() override;
		static std::shared_ptr<FunctionDecl> Parse(bool ext = false);
	};

	// class for matching variable declaration.
	class FieldDecl :public Statement
	{
		bool constant;
		std::wstring name, type;
		std::shared_ptr<Expr> value;
	public:
		void Gen() override;
		static std::shared_ptr<FieldDecl> Parse(bool is_const);
	};

	// class for matching class declaration.
	class ClassDecl final :public Declaration
	{
	public:
		std::wstring name;
		std::vector<std::wstring> fields;
		std::vector<std::wstring> types;
		std::vector<std::shared_ptr<FunctionDecl>> functions;
		static std::shared_ptr<ClassDecl> Parse();
		void Gen() override;
		void GenHeader() override;
	};
	
	// class for if statement.
	class If :public Statement
	{
		std::shared_ptr<Expr> condition;
		std::shared_ptr<Statement> stmts;
		std::shared_ptr<Statement> else_stmts;
	public:
		static std::shared_ptr<If> Parse();
		void Gen() override;
	};

	class Empty :public Statement
	{
		std::shared_ptr<Expr> value;
	public:
		static std::shared_ptr<Empty> Parse();
		void Gen() override;
	};

	class Throw :Statement
	{
		std::shared_ptr<Expr> value;
	public:
		static std::shared_ptr<Throw> Parse();
		void Gen() override;
	};

	class Return :public Statement
	{
		std::shared_ptr<Expr> value;
	public:
		static std::shared_ptr<Return> Parse();
		void Gen() override;
	};

	class Break final :Statement
	{
	public:
		static std::shared_ptr<Break> Parse();
		void Gen() override;
	};

	class Continue final :Statement
	{
	public:
		static std::shared_ptr<Continue> Parse();
		void Gen() override;
	};

	class Import final :Statement
	{
	public:
		static std::shared_ptr<Import> Parse();
		void Gen() override;
	};

	class While final :public Statement
	{
	public:
		std::shared_ptr<Expr> condition;
		std::shared_ptr<Statement> stmts;
		static std::shared_ptr<While> Parse();
		void Gen() override;
	};

	class Do final :public Statement
	{
	public:
		std::shared_ptr<Expr> condition;
		std::shared_ptr<Statement> stmts;
		static std::shared_ptr<Do> Parse();
		void Gen() override;
	};

	class For final :public Statement
	{
	public:
		std::shared_ptr<Expr> condition;
		std::shared_ptr<Statement> stmts;
		static std::shared_ptr<For> Parse();
		void Gen() override;
	};

//********************************************************************************************************
//*							ToString
//********************************************************************************************************


	// implement all the ToString function for Expression nodes below.
	inline void Factor::ToString() { *debugger::out << "[Factor]"; }
	inline void NumberConst::ToString()
	{

		switch (type)
		{
		case K_int:*debugger::out << "[" << static_cast<int>(value) << "]"; return;
		case K_float:*debugger::out << "[" << value << "]", value; return;
		case K_double:*debugger::out << "[" << value << "]"; return;
		default:*debugger::out << "[" << Token::Name(type) << "]"; return;
		}
	}
	inline void String::ToString() { *debugger::out << "[\"" << value << "\"]"; }
	inline void Boolean::ToString() { *debugger::out << "[" << (value ? "true" : "false") << "]"; }
	inline void Field::ToString() { *debugger::out << "[" << names[0] << "]"; }
	inline void FuncCall::ToString()
	{
		*debugger::out << "[CALL " << names[0] << " ( ";
		for (auto& arg : args)
		{
			arg->ToString();
			*debugger::out << ",";
		}
		*debugger::out << " )]\n";
	}
	inline void Unary::ToString() { *debugger::out << "<" << Token::Name(op) << ">"; expr->ToString(); }
	inline void Binary::ToString() { *debugger::out << "("; LHS->ToString(); *debugger::out << " " << Token::Name(op) << " "; RHS->ToString(); *debugger::out << ")"; }
	inline void Ternary::ToString()
	{
		*debugger::out << "[";
		a->ToString();
		*debugger::out << "?";
		b->ToString();
		*debugger::out << ":";
		c->ToString();
		*debugger::out << "]";
	}




	//********************************************************************************************************
	//*							Parser
	//********************************************************************************************************

	inline std::shared_ptr<Statement> Statements::Parse()
	{
		while (Check( NewLine)) { Next(); VERIFY }
		if (Check( '}')) return nullptr;
		const auto left = Statement::Parse(); VERIFY
		const auto right = Statements::Parse(); VERIFY
		if (right == nullptr)return left;
		return std::make_shared<Statements>(left, right);
	}
	
	inline std::shared_ptr<FuncCall> FuncCall::Parse(std::vector <std::wstring> f)
	{
		auto func_call = std::make_shared<FuncCall>(f);
		Next();												VERIFY
		while (token->type != ')') {
			func_call->args.push_back(Binary::Parse());		VERIFY
			if (Check( ',')) Match(',');					VERIFY
		}
		Match(')');										VERIFY
		return func_call;
	}

	inline std::shared_ptr<Expr> Unary::ParsePostfix()
	{
		const auto factor = ParsePrefix();
																VERIFY
		switch (token->type)
		{

		case Inc:
		case Dec:
			Next();												VERIFY
			return std::make_shared<Unary>(factor, token->type, false);
		default:
			return factor;
		}
	}

	inline std::shared_ptr<Expr> Unary::ParsePrefix()
	{
		switch (token->type)
		{
		case NewLine:
			AlertNewline();
		    Alert(L"unexpected EndOfLine");
			return nullptr;
		case '-':
		case '!':
		case Inc:
		case Dec: {
			Next();											VERIFY
				const auto parsed = Parse();						VERIFY
				return std::make_shared<Unary>(parsed, token->type, true);
		}
		default:
			return Factor::Parse();	
		}

	}
	
	inline std::shared_ptr<Expr> Ternary::Parse()
	{
		const auto a = Binary::Sub7();		VERIFY
		if (token->type != '?')return a;
		Next();								VERIFY
		const auto b = Binary::Sub7();		VERIFY
		Match(':');						VERIFY
		const auto c = Binary::Sub7();		VERIFY
		return std::make_shared<Ternary>(a, b, c);
	}

	inline std::shared_ptr<Expr> Factor::Parse()
	{
		
		std::shared_ptr<Expr> factor;
		switch (token->type)
		{
		case '(': {
			Next();
			// const auto point = src;
			// auto is_lambda = false;
			// Find('(', ')');
			// Next();												VERIFY
			// PRINT("ahead token is :"<<Token::Name(token->type)<<"\n", );
			// is_lambda = token->type == Arrow;
			// 	
			// PRINT("isLambda:%d   ", is_lambda);
			// src = point;
			// Next();												VERIFY
			// PRINT("current token is :"<<Token::Name(token->type)<<"\n");
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
			auto str = std::make_shared<String>(string_val);
			Next(); VERIFY
			return str;
		}

		case Num:
		{
			const auto ty = token->value;
			Next(); VERIFY
			return std::make_shared<NumberConst>(number_val, ty);
		}
		case K_true:
			Next(); VERIFY
			return std::make_shared<Boolean>(true);
		case K_false:
			Next(); VERIFY
			return std::make_shared<Boolean>(false);
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
			if (Check( '('))return FuncCall::Parse(names);
			else if (Check('['))
			{
				VERIFY
			}
			else return std::make_shared<Field>(names);
		}
		default:
			Alert((std::wstringstream()<<"unexpected \"" << Token::Name(token->type) << "\" ").str());
			return nullptr;
		}
    }


	
	inline std::shared_ptr<FuncParam> FuncParam::Parse()
	{
		auto param = std::make_shared<FuncParam>();
		while (token->type != ')') {
			param->size++;
			if (CheckType())
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
			if (Check(',')) Match(',');	VERIFY
		}
		return param;
	}
	
	inline std::shared_ptr<FunctionDecl> FunctionDecl::Parse(const bool ext)
	{
		auto function = std::make_shared<FunctionDecl>();
		function->is_extern = ext;
		if (Check(K_dfunc))function->differentiable = true;
		else if (Check(K_kernal))function->kernal = true;
		Next();


		VERIFY
		if (Check(Id))
		{
			function->name = string_val;
			Match(Id); VERIFY
		}
		Match('('); VERIFY
		function->args = FuncParam::Parse();
		Match(')'); VERIFY

		function->return_type = '0';
		if (Check(':'))
		{
			Next(); VERIFY
			if (CheckType())
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

		while (Check(NewLine))Next(); VERIFY
		if (ext)
		{
			*out<<"[Parsed] Extern function declaration\n";
			return function;
		}
		*out <<"[Parsed] Function declaration\n";
		Match('{'); VERIFY

		function->statements = Statements::Parse();
		while (Check(NewLine))Next(); VERIFY

		Match('}'); VERIFY
		*out<<"[Parsed] Function end\n";
		return function;
	}
	
	inline std::shared_ptr<FieldDecl> FieldDecl::Parse(const bool is_const)
	{
		auto let = std::make_shared<FieldDecl>();
		let->constant = is_const;
		Next(); VERIFY
		let->name = string_val;
		Match(Id);
		if (Check(':'))
		{
			Next(); VERIFY
			if (CheckType())
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

		if (Check(';')) Next();
		else Match(NewLine);
		VERIFY
		// PRINT("[Parsed] %s field declaration\n", is_const ? "Constant" : "Variable");let->value->ToString();PRINT("\n");
		return let;
	}

	inline std::shared_ptr<ClassDecl> ClassDecl::Parse()
	{
		auto instance = std::make_shared<ClassDecl>();
		Next();													VERIFY
		instance->name = string_val;
		Match(Id);											VERIFY
		while (Check(NewLine))Next();							VERIFY
		Match('{');											VERIFY
		
		
		while (true) {
			while (Check(NewLine))Next();							VERIFY
			if (token->type == '}')break;
			switch (token->type)
			{
			case K_init:
			case K_deinit:
			case K_func:
			case K_dfunc:
			case K_kernal:
				instance->functions.push_back(FunctionDecl::Parse()); VERIFY
				break;
			default:
				instance->fields.push_back(string_val);
				Match(Id); VERIFY
				Match(':');

				if (CheckType())
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

				if (Check(';')) Match(';');
				else Match(NewLine);
				while (Check(NewLine))Next();					VERIFY
			}
		}
		Match('}'); VERIFY
		return instance;
	}
	
	inline std::shared_ptr<If> If::Parse()
	{
		const auto instance = std::make_shared<If>();
		Next();
		Match('(');
		instance->condition = Binary::Parse();
		Match(')');
		if(Check('{'))
		{
			Next();
			instance->stmts = Statements::Parse();
			Match('}');
		}
		else instance->stmts = Statement::Parse();
		

		if (Check(K_else))
		{
			Next();
			if (Check('{'))
			{
				Next();
				instance->else_stmts = Statements::Parse();
				Match('}');
			}
			else instance->else_stmts = Statement::Parse();
		}
		else if (Check(K_elif))instance->else_stmts = Parse();
	
		return instance;
	}

	inline std::shared_ptr<While> While::Parse()
	{
		const auto instance = std::make_shared<While>();
		Next();
		Match('(');
		instance->condition = Binary::Parse();
		Match(')');
		if (Check('{'))
		{
			Next();
			instance->stmts = Statements::Parse();
			Match('}');
		}
		else instance->stmts = Statement::Parse();
		return instance;
	}
 
	inline std::shared_ptr<For> For::Parse()
	{
		const auto instance = std::make_shared<For>();
		Next();
		Match('(');
		instance->condition = Binary::Parse();
		Match(')');
		if (Check('{'))
		{
			Next();
			instance->stmts = Statements::Parse();
			Match('}');
		}
		else instance->stmts = Statement::Parse();
		return instance;
	}
	
	inline std::shared_ptr<Do> Do::Parse()
	{
		const auto instance = std::make_shared<Do>();
		Next();
		if (Check('{'))
		{
			Next();
			instance->stmts = Statements::Parse();
			Match('}');
		}
		else instance->stmts = Statement::Parse();

		Match(K_do);
		Match('(');
		instance->condition = Binary::Parse();
		Match(')');
		return instance;
	}
	inline std::shared_ptr<Throw> Throw::Parse()
	{
		Next();
		auto instance = std::make_shared<Throw>();
		instance->value = Factor::Parse();
		return instance;
	}
	
	inline std::shared_ptr<Empty> Empty::Parse()
	{
	
		auto instance = std::make_shared<Empty>();
		instance->value = Binary::Parse(); VERIFY
		if (token->type == ';')Next();
		else Match(NewLine);
								VERIFY
		// PRINT("[Parsed] Expression \n");instance->value->ToString();
		return instance;
	}

	inline std::shared_ptr<Return> Return::Parse()
	{
		Next();
		auto instance = std::make_shared<Return>(); 
		if (token->type == ';'||token->type ==NewLine)
		{
			instance->value = nullptr;
			Next();
			return instance;
		}
		instance->value = Binary::Parse();
		if (token->type == ';')Next();
		else Match(NewLine);
		*out <<"[Parsed] Return Statement\n";
		return instance;
	}

	inline std::shared_ptr<Import> Import::Parse()
	{
		Next();
		auto instance = std::make_shared<Import>();
		return instance;
	}
	
	inline std::shared_ptr<Break> Break::Parse()
	{
		Next();
		auto instance = std::make_shared<Break>();
		return instance;
	}
	
	inline std::shared_ptr<Continue> Continue::Parse()
	{
		Next();
		auto instance = std::make_shared<Continue>();
		return instance;
	}

	inline std::shared_ptr<Statement> Statement::Parse()
	{
		while (Check(NewLine))Next(); VERIFY
			switch (token->type)
			{
			case K_let:		return FieldDecl::Parse(true);
			case K_var:		return FieldDecl::Parse(false);
			case K_if:		return If::Parse();
			case K_return:	return Return::Parse();
			case K_do:		return For::Parse();
			case K_while:	return While::Parse();
			case K_for:		return For::Parse();
			default:
				return Empty::Parse();
			}
	}

	//***************** Driver ***************
	// Program is a class that will match and store all the codes, it is the root of AST
	class Program
	{
		std::vector<std::shared_ptr<Statement>> statements;
		std::vector<std::shared_ptr<Declaration>> declarations;
		void ParseSingle() {
			switch (token->type)
			{
			case NewLine:Next(); break;
			case K_func:
			case K_dfunc:
			case K_kernal:	declarations.push_back(FunctionDecl::Parse());break;
			case K_extern:	declarations.push_back(FunctionDecl::Parse(true));break;
			case K_import:	Import::Parse(); break;
			case K_class:	declarations.push_back(ClassDecl::Parse()); break;
			default: statements.push_back(Statement::Parse());
			}
			while (error_occurred)
			{
				error_occurred = false;
				MoveLine();
				Next();
			}
		}
	public:
		static std::shared_ptr<Program> Parse()
		{
			auto program = std::make_shared<Program>();
			while (peek > 0 && token != nullptr) program->ParseSingle();
			return program;
		}
		void Gen();
	};

	static std::shared_ptr<Program> Parse()
	{
		Next();
		if (!only_tokenize)return Program::Parse();
		while (peek > 0 && token!=nullptr) { *out <<"["<< Token::Name(token->type)<<"] ";	if (Check(NewLine) || Check(';'))* out <<"\n";Next();}return nullptr;
	}
};



#endif
