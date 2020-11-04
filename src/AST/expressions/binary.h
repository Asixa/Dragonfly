#ifndef BINARY_H
#define BINARY_H
#include "AST/expressions/expr.h"
#include "frontend/lexer.h"
#include "AST/expressions/unary.h"
#include "AST/expressions/ternary.h"
#include "field.h"
using namespace frontend;
namespace AST {
	namespace expr {
		// Expression node for all binary expressions
		class Binary final : public Expr {
			static std::map<std::string, std::function<llvm::Value* (llvm::Value*, llvm::Value*, std::shared_ptr<DFContext>)>> gens;
		public:
			int op;
			std::shared_ptr<Expr> LHS, RHS;
			Binary(std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs, int op) : op(op), LHS(lhs), RHS(rhs) {}
			void ToString() override;
			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, bool is_ptr) override;
			llvm::Value* Gen2(std::shared_ptr<DFContext>);
			Binary() {}
			Binary(const std::string lhs, const std::string rhs, const int o) :LHS(std::make_shared<Field>(lhs)), RHS(std::make_shared<Field>(rhs)), op(o) {}
			// This part might be hard to understand.
			// PARSE is a Macro to generate functions.
			// When parsing binary Expressions, This start from top to bottom.
			// The order of operators is important. shown below as Sub1 ~ Sub7.
#define PARSE(name,func,condition,check)                            \
        static std::shared_ptr<Expr> name() {                       \
            auto left = func();                                     \
            check(condition) {                                      \
                auto op = Lexer::token->type;Lexer::Next();         \
                if(Lexer::Check(NewLine)) {                         \
                    Debugger::CatchNewline();                       \
                    Debugger::Error("unexpected EndOfLine");       \
                    return nullptr;                                 \
                }                                                   \
                left= std::make_shared<Binary>(left, func(), op);   \
            } return left;}


			PARSE(Sub1, Unary::Parse, Lexer::Check('*') || Lexer::Check('/') || Lexer::Check('%'), while)
				PARSE(Sub2, Sub1, Lexer::Check('+') || Lexer::Check('-'), while)
				PARSE(Sub3, Sub2, Lexer::Check(Shl) || Lexer::Check(Shr), while)
				PARSE(Sub4, Sub3, Lexer::Check('>') || Lexer::Check('<') || Lexer::Check(Ge) || Lexer::Check(Le), if)
				PARSE(Sub5, Sub4, Lexer::Check(Eq) || Lexer::Check(Ne), while)
				PARSE(Sub6, Sub5, Lexer::Check('|') || Lexer::Check('^') || Lexer::Check('&'), while)
				PARSE(Sub7, Sub6, Lexer::Check(Or) || Lexer::Check(And), while)
				PARSE(Parse, Ternary::Parse,
					Lexer::Check({ '=',AddAgn,SubAgn ,SubAgn,DivAgn,MulAgn,ModAgn,ShlAgn,ShrAgn,BAndAgn,BXORAgn,BORAgn }), while)

#undef  PARSE
		};
	}
}
#endif