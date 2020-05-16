#include "AST/expressions/binary.h"
#include "codegen.h"

namespace parser {
	void Binary::ToString() {
		*Debugger::out << "(";
		LHS->ToString();
		*Debugger::out << " " << Lexer::Token::Name(op) << " ";
		RHS->ToString();
		*Debugger::out << ")";
	}
	llvm::Value* Binary::Gen(int cmd) {

		const auto load_ptr = op == '=' || op >= AddAgn;
		auto lhs = LHS->Gen(load_ptr);
		auto rhs = RHS->Gen(load_ptr);
		if (!lhs || !rhs)return CodeGen::LogErrorV("  operands is NULL \n");;


		auto type = lhs->getType()->getTypeID();
		const auto ltype = lhs->getType()->getTypeID();
		const auto rtype = rhs->getType()->getTypeID();

		if (ltype != rtype) {
			if (ltype == llvm::Type::IntegerTyID) {
				if (rtype == llvm::Type::FloatTyID) {
					type = llvm::Type::FloatTyID;
					lhs = CodeGen::builder.CreateUIToFP(lhs, llvm::Type::getFloatTy(CodeGen::the_context));
				}
				else if (rtype == llvm::Type::DoubleTyID) {
					type = llvm::Type::DoubleTyID;
					lhs = CodeGen::builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(CodeGen::the_context));
				}
			}
			if (rtype == llvm::Type::IntegerTyID) {
				if (ltype == llvm::Type::FloatTyID) {
					type = llvm::Type::FloatTyID;
					rhs = CodeGen::builder.CreateUIToFP(lhs, llvm::Type::getFloatTy(CodeGen::the_context));
				}
				else if (ltype == llvm::Type::DoubleTyID) {
					type = llvm::Type::DoubleTyID;
					rhs = CodeGen::builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(CodeGen::the_context));
				}
			}
		}

		switch (op) {

#define BASIC(f,i)[&](){                                                                                     \
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)                        \
				return CodeGen::builder.Create##f(lhs, rhs, #f"_tmp");                                  \
			if (type == llvm::Type::IntegerTyID)                                                        \
				return CodeGen::builder.Create##i(lhs, rhs, #i"_tmp");                                  \
			return CodeGen::LogErrorV( std::strcat(const_cast<char*>(Lexer::Token::Name(op)), " operation cannot apply on Non-number operands\n")); }();
#define BITWISE(f)[&](){                                                                                                                             \
			if (type == llvm::Type::IntegerTyID)return CodeGen::builder.Create##f(lhs, rhs, "and_tmp");                                         \
			return CodeGen::LogErrorV(std::strcat(const_cast<char*>(Lexer::Token::Name(op)), " operation cannot apply on Integer operands\n")); \
		}();

		case '+':return BASIC(FAdd, Add)
		case '-':return BASIC(FSub, Sub)
		case '*':return BASIC(FMul, Mul)
		case '%':return BASIC(FRem, SRem)
		case '<':return BASIC(FCmpULT, ICmpULT)
		case '>':return BASIC(FCmpULT, ICmpULT)
		case Le:return BASIC(FCmpULE, ICmpULE)
		case Ge:return BASIC(FCmpUGE, ICmpUGE)
		case Eq:return BASIC(FCmpUEQ, ICmpEQ)
		case Ne:return BASIC(FCmpUNE, ICmpNE)

		case And:
		case '&': return BITWISE(And)
		case '^': return BITWISE(Xor)
		case Or:
		case '|': return BITWISE(Or)
		case Shr: return BITWISE(AShr)
		case Shl: return BITWISE(Shl)

		case '/': {
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)
				return CodeGen::builder.CreateFDiv(
					lhs, rhs, "FDiv""_tmp");
			if (type == llvm::Type::IntegerTyID)
				return CodeGen::builder.CreateFDiv(
					CodeGen::builder.CreateCast(llvm::Instruction::SIToFP, lhs, llvm::Type::getDoubleTy(CodeGen::the_context)),
					CodeGen::builder.CreateCast(llvm::Instruction::SIToFP, rhs, llvm::Type::getDoubleTy(CodeGen::the_context)),
					"FDiv""_tmp");
			return CodeGen::LogErrorV(" ""'/'"" operation cannot apply on Non-number operands\n");
		}
		case BAndAgn: { }
		case BXORAgn: { }
		case BORAgn: { }

		case '=': {
			if (lhs->getType()->getTypeID() != llvm::Type::PointerTyID)
				return CodeGen::LogErrorV(
					"cannot reassign a constant\n");
			auto rhv = rhs;
			if (rhs->getType()->getTypeID() != lhs->getType()->getPointerElementType()->getTypeID())
				rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID
				? CodeGen::AlignLoad(CodeGen::builder.CreateLoad(rhs, "rhs"))
				: rhs;
			CodeGen::AlignStore(CodeGen::builder.CreateStore(rhv, lhs));
			return lhs;
		}

#define ASSGIN(x){																											\
				auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID ? CodeGen::AlignLoad(CodeGen::builder.CreateLoad(rhs,"rhs")) : rhs;			\
				if (ltype == llvm::Type::PointerTyID)																			\
				{																										\
					type = lhs->getType()->getPointerElementType()->getTypeID();										\
					const auto lhsv = CodeGen::AlignLoad(CodeGen::builder.CreateLoad(lhs,"lhs"));															\
					return CodeGen::AlignStore(CodeGen::builder.CreateStore(x, lhs));						\
				}																										\
				return CodeGen::LogErrorV(" cannot reassign a constant\n");														\
			}
#define BASIC_ASSGIN(a,b,c,d)case a:																					\
			{																											\
				auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID ? CodeGen::AlignLoad(CodeGen::builder.CreateLoad(rhs,"rhs")) : rhs;			\
				if (type == llvm::Type::PointerTyID)																			\
				{																										\
					type = lhs->getType()->getPointerElementType()->getTypeID();										\
					const auto lhsv = CodeGen::AlignLoad(CodeGen::builder.CreateLoad(lhs,"lhs"));															\
					if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)											\
						return CodeGen::AlignStore(CodeGen::builder.CreateStore(CodeGen::builder.Create##b(lhsv, rhv, #b"_tmp"), lhs));						\
					if (type == llvm::Type::IntegerTyID)																		\
						return CodeGen::AlignStore(CodeGen::builder.CreateStore(CodeGen::builder.Create##c(lhsv, rhv, #c"_tmp"), lhs));						\
					return CodeGen::LogErrorV(" "#d" operation cannot apply on Non-number variables\n");							\
				}																										\
				return CodeGen::LogErrorV(" cannot reassign a constant\n");														\
			}

				  // #define  BITWISE_ASSGIN(a,b,c,d)case a: {\
	              // 			if (type == Type::IntegerTyID)\
	              // 				lhs = CodeGen::builder.Create##c(lhs, rhs, #c"_tmp"); \
	              // 			else return CodeGen::LogErrorV(" "#d" operation cannot apply on Non-number variables\n");\
	              // 			return lhs;}

				  BASIC_ASSGIN(AddAgn, FAdd, Add, +=)
					  BASIC_ASSGIN(SubAgn, FSub, Sub, -=)
					  BASIC_ASSGIN(MulAgn, FMul, Mul, *=)
					  BASIC_ASSGIN(ModAgn, FRem, SRem, %=)

		case DivAgn: {
					  auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID
						  ? CodeGen::AlignLoad(CodeGen::builder.CreateLoad(rhs, "rhs"))
						  : rhs;
					  if (type == llvm::Type::PointerTyID) {
						  type = lhs->getType()->getPointerElementType()->getTypeID();
						  const auto lhsv = CodeGen::AlignLoad(CodeGen::builder.CreateLoad(lhs, "lhs"));
						  if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)
							  return CodeGen::AlignStore(
								  CodeGen::builder.CreateStore(CodeGen::builder.CreateFDiv(lhsv, rhv, "FDiv""_tmp"), lhs));
						  if (type == llvm::Type::IntegerTyID) {
							  const auto lhv_d = CodeGen::builder.CreateCast(llvm::Instruction::SIToFP, lhsv,
								  llvm::Type::getDoubleTy(CodeGen::the_context));
							  const auto rhv_d = CodeGen::builder.CreateCast(llvm::Instruction::SIToFP, rhv,
								  llvm::Type::getDoubleTy(CodeGen::the_context));
							  const auto div = CodeGen::builder.CreateFDiv(lhv_d, rhv_d, "div_tmp");
							  const auto div_i = CodeGen::builder.CreateCast(llvm::Instruction::FPToUI, div,
								  llvm::Type::getInt32Ty(CodeGen::the_context));
							  return CodeGen::AlignStore(CodeGen::builder.CreateStore(div_i, lhs));
						  }


						  return CodeGen::LogErrorV(" ""/="" operation cannot apply on Non-number variables\n");
					  }
					  return CodeGen::LogErrorV(" cannot reassign a constant\n");
				  }
		default:
			return CodeGen::LogErrorV("invalid binary operator");

		}
	}
}
