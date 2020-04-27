// Copyright 2019 The Dragonfly Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>

namespace parser {

    // Base Class for Expressions
    class Expr {
    public:
        virtual ~Expr() = default;
        virtual void ToString() = 0;
        virtual llvm::Value* Gen(int cmd = 0) = 0;
    };
	
    // Base Class for Statements
    class Statement {
    public:
        virtual ~Statement() = default;
        virtual void Gen() = 0;
        static std::shared_ptr<Statement> Parse();
    };
	
    // Base Class for Declaration, which will generate codes before Statments
    class Declaration : public Statement {
    public:
        virtual void GenHeader() = 0;
    };
	
    // 'Statements' Class will match all statements in a binary-tree structure
    class Statements final : public Statement {
        std::shared_ptr<Statement> stmt1;
        std::shared_ptr<Statement> stmt2;
    public:
        Statements(std::shared_ptr<Statement> a, std::shared_ptr<Statement> b): stmt1(a), stmt2(b) {}
        static std::shared_ptr<Statement> Parse();
        void Gen() override;
    };

    //********************   Expressions ***********************

    // Expression node for literal numbers.
    class NumberConst final : public Expr {
    public:
        int type;
        double value;
        explicit NumberConst(const double d, const int t) : value(d) { type = t; }
        void ToString() override;
        llvm::Value* Gen(const int cmd = 0) override;
    };
	
    // Expression node for literal booleans.
    class Boolean final : public Expr {
    public:
        void ToString() override;
        llvm::Value* Gen(const int cmd = 0) override;
        bool value;
        explicit Boolean(const bool d) : value(d) {}
    };

    // Expression node for literal strings.
    class String final : public Expr {
    public:
        void ToString() override;
        llvm::Value* Gen(const int cmd = 0) override;
        std::wstring value;
        String(std::wstring d) : value(std::move(d)) {}
    };

    // Expression node for Lambda expression.
    class Lambda final : public Expr {
    public:
        void ToString() override;
        llvm::Value* Gen(const int cmd = 0) override;
        static std::shared_ptr<Lambda> Parse() { return nullptr; }
    };
	
    // Expression node for variable or fields.
    class Field final : public Expr {
    public:
        void ToString() override;
        llvm::Value* Gen(const int cmd = 0) override;
        std::vector<std::wstring> names;
        explicit Field(std::vector<std::wstring> d) : names(d) {}
    };
	
    // Expression node for function calls.
    class FuncCall final : public Expr {
    public:
        void ToString() override;
        llvm::Value* Gen(const int cmd = 0) override;
        std::vector<std::wstring> names;
        std::vector<std::shared_ptr<Expr>> args;
        explicit FuncCall(std::vector<std::wstring> d) : names(d) {}
        static std::shared_ptr<FuncCall> Parse(std::vector<std::wstring> f);
    };
	
    // Expression node for All general factors (including all expression nodes above)
    class Factor final : public Expr {
    public:
        void ToString() override;
        llvm::Value* Gen(const int cmd = 0) override;
        Lexer::Token* tok;
        explicit Factor(Lexer::Token* t): tok(t) {}
        static std::shared_ptr<Expr> Parse();
    };
	
    // Expression node for factors with prefix or postfix)
    class Unary final : public Expr {
    public:
        void ToString() override;
        llvm::Value* Gen(const int cmd = 0) override;
        int op;
        bool prefix;
        std::shared_ptr<Expr> expr;
        Unary(std::shared_ptr<Expr> expr, const int op, bool pre): op(op), expr(std::move(expr)), prefix(pre) {}
        static std::shared_ptr<Expr> ParsePrefix();
        static std::shared_ptr<Expr> ParsePostfix();
        static std::shared_ptr<Expr> Parse();
    };

    // Expression node for Ternary expression like a?b:c
    class Ternary final : public Expr {
        std::shared_ptr<Expr> a;
        std::shared_ptr<Expr> b;
        std::shared_ptr<Expr> c;
    public:
        void ToString() override;
        llvm::Value* Gen(const int cmd = 0) override;
        Ternary(std::shared_ptr<Expr> x, std::shared_ptr<Expr> y, std::shared_ptr<Expr> z) : a(x), b(y), c(z) {}
        static std::shared_ptr<Expr> Parse();
    };
	
    // Expression node for all binary expressions
    class Binary final : public Expr {
    public:
        int op;
        std::shared_ptr<Expr> LHS,RHS;
        Binary(std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs, int op): op(op), LHS(lhs), RHS(rhs) {}
		void ToString() override;
		llvm::Value* Gen(const int cmd = 0) override;

 // This part might be hard to understand.
 // PARSE is a Macro to generate functions.
 // When parsing binary Expressions, This start from top to bottom.
 // The order of operators is important. shown below as Sub1 ~ Sub7.
#define PARSE(name,func,condition,check)                            \
        static std::shared_ptr<Expr> name() {                       \
            auto left = func();VERIFY                               \
            check(condition) {                                      \
                auto op = Lexer::token->type;Lexer::Next();         \
                if(Lexer::Check(NewLine)) {                         \
                    Debugger::AlertNewline();                       \
                    Debugger::Alert(L"unexpected EndOfLine");       \
                    return nullptr;                                 \
                }   VERIFY                                          \
                left= std::make_shared<Binary>(left, func(), op);   \
            } return left;}

        PARSE(Sub1, Unary::Parse, Lexer::Check('*') || Lexer::Check( '/') || Lexer::Check( '%'),while)

        PARSE(Sub2, Sub1, Lexer::Check( '+') || Lexer::Check( '-'), while)
        PARSE(Sub3, Sub2, Lexer::Check( Shl) || Lexer::Check( Shr), while)
        PARSE(Sub4, Sub3, Lexer::Check( '>') || Lexer::Check( '<') || Lexer::Check( Ge) || Lexer::Check( Le), if)
        PARSE(Sub5, Sub4, Lexer::Check( Eq) || Lexer::Check( Ne), while)
        PARSE(Sub6, Sub5, Lexer::Check( '|') || Lexer::Check( '^') || Lexer::Check( '&'), while)
        PARSE(Sub7, Sub6, Lexer::Check( Or) || Lexer::Check( And), while)
        PARSE(Parse, Ternary::Parse,
			Lexer::Check({ '=',AddAgn,SubAgn ,SubAgn,DivAgn,MulAgn,ModAgn,ShlAgn,ShrAgn,BAndAgn,BXORAgn,BORAgn }),while)
#undef  PARSE
    };

    //**********   End of Expressions ****************


    //************************   Statements  ******************************

    // class for matching function declaration parameters; 
    class FuncParam {
    public:
        int size;
        bool isVarArg = false;
        std::vector<std::wstring> names, types;
        static std::shared_ptr<FuncParam> Parse();
    };

    // class for matching function definition.
    class FunctionDecl final : public Declaration {
    public:
        bool differentiable = false,
             kernal = false,
             is_extern = false;
        std::wstring name;
        llvm::StructType* self_type;
        std::wstring return_type;
        std::shared_ptr<FuncParam> args;
        std::shared_ptr<Statement> statements;

        void SetInternal(const std::wstring structname, llvm::StructType* type) {
            name = structname + L"." + name;
            self_type = type;
        }

        void Gen() override;
        void GenHeader() override;
        static std::shared_ptr<FunctionDecl> Parse(bool ext = false);
    };

    // class for matching variable declaration.
    class FieldDecl : public Statement {
        bool constant;
        std::wstring name, type;
        std::shared_ptr<Expr> value;
    public:
        void Gen() override;
        static std::shared_ptr<FieldDecl> Parse(bool is_const);
    };

    // class for matching class declaration.
    class ClassDecl final : public Declaration {
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
    class If : public Statement {
        std::shared_ptr<Expr> condition;
        std::shared_ptr<Statement> stmts;
        std::shared_ptr<Statement> else_stmts;
    public:
        static std::shared_ptr<If> Parse();
        void Gen() override;
    };

    class Empty : public Statement {
        std::shared_ptr<Expr> value;
    public:
        static std::shared_ptr<Empty> Parse();
        void Gen() override;
    };

    class Throw : Statement {
        std::shared_ptr<Expr> value;
    public:
        static std::shared_ptr<Throw> Parse();
        void Gen() override;
    };

    class Return : public Statement {
        std::shared_ptr<Expr> value;
    public:
        static std::shared_ptr<Return> Parse();
        void Gen() override;
    };

    class Break final : Statement {
    public:
        static std::shared_ptr<Break> Parse();
        void Gen() override;
    };

    class Continue final : Statement {
    public:
        static std::shared_ptr<Continue> Parse();
        void Gen() override;
    };

    class Import final : Statement {
    public:
        static std::shared_ptr<Import> Parse();
        void Gen() override;
    };

    class While final : public Statement {
    public:
        std::shared_ptr<Expr> condition;
        std::shared_ptr<Statement> stmts;
        static std::shared_ptr<While> Parse();
        void Gen() override;
    };

    class Do final : public Statement {
    public:
        std::shared_ptr<Expr> condition;
        std::shared_ptr<Statement> stmts;
        static std::shared_ptr<Do> Parse();
        void Gen() override;
    };

	class Switch final : public Statement {
	public:
		std::shared_ptr<Expr> value;
		std::shared_ptr<Statement> stmts;
		static std::shared_ptr<Switch> Parse();
		void Gen() override;
	};

    class For final : public Statement {
    public:
        std::shared_ptr<Expr> condition;
        std::shared_ptr<Statement> stmts;
        static std::shared_ptr<For> Parse();
        void Gen() override;
    };


    class Program {
        std::vector<std::shared_ptr<Statement>> statements;
        std::vector<std::shared_ptr<Declaration>> declarations;
		void ParseSingle();
    public:
		static std::shared_ptr<Program> Parse();
        void Gen();
    };

	std::shared_ptr<Program> Parse();

};

#endif
