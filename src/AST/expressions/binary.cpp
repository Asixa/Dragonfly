#include "AST/expressions/binary.h"
#include "LLVM/context.h"

namespace AST {
	using namespace expr;
	void Binary::ToString() {
		*Debugger::out << "(";
		LHS->ToString();
		*Debugger::out << " " << Lexer::Token::Name(op) << " ";
		RHS->ToString();
		*Debugger::out << ")";
	}

	std::shared_ptr<AST::Type> Binary::Analysis(std::shared_ptr<DFContext>) { return nullptr; }
	llvm::Value* Binary::Gen2(std::shared_ptr<DFContext> ctx, int cmd) {
        const auto lhs = LHS->Gen(ctx, cmd),rhs = RHS->Gen(ctx, cmd);
		const auto lhs_type = lhs->getType()->getTypeID(),
	               rhs_type = rhs->getType()->getTypeID();
		auto op_name = std::string(Lexer::Token::Name(op));
        const auto assign = op_name.size() > 1 && (op_name.back() == '=' && op> Ge);  // check if it is += -= /= *= ...
		if (assign) op_name.pop_back();
        const auto name=ctx->GetStructName(lhs)+op_name+ctx->GetStructName(rhs);

        if(gens.find(name)!=gens.end()) gens[name](lhs, rhs, ctx);
		else {
			auto operator_func=ctx->GetFunction(name);
            if(operator_func) {
				// ctx->builder->CreateCall(operator_func, args_v);
            }
		}

		if (op == '=' || assign)
			return ctx->builder->CreateStore(lhs,rhs);
	}

    llvm::Value* Binary::Gen(std::shared_ptr<DFContext> ctx,int cmd) {

		const auto load_ptr = op == '=' || op >= AddAgn;
		auto lhs = LHS->Gen(ctx,load_ptr);
		auto rhs = RHS->Gen(ctx,load_ptr);
		if (!lhs || !rhs)return Debugger::ErrorV("operands is NULL",line,ch);;


		auto type = lhs->getType()->getTypeID();
		const auto ltype = lhs->getType()->getTypeID();
		const auto rtype = rhs->getType()->getTypeID();

		if (ltype != rtype) {
			if (ltype == llvm::Type::IntegerTyID) {
				if (rtype == llvm::Type::FloatTyID) {
					type = llvm::Type::FloatTyID;
					lhs = ctx->builder->CreateUIToFP(lhs, llvm::Type::getFloatTy(ctx->context));
				}
				else if (rtype == llvm::Type::DoubleTyID) {
					type = llvm::Type::DoubleTyID;
					lhs = ctx->builder->CreateUIToFP(lhs, llvm::Type::getDoubleTy(ctx->context));
				}
			}
			if (rtype == llvm::Type::IntegerTyID) {
				if (ltype == llvm::Type::FloatTyID) {
					type = llvm::Type::FloatTyID;
					rhs = ctx->builder->CreateUIToFP(lhs, llvm::Type::getFloatTy(ctx->context));
				}
				else if (ltype == llvm::Type::DoubleTyID) {
					type = llvm::Type::DoubleTyID;
					rhs = ctx->builder->CreateUIToFP(lhs, llvm::Type::getDoubleTy(ctx->context));
				}
			}
		}
#define BASIC(f,i)[&](){                                                                                     \
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)                        \
				return ctx->builder->Create##f(lhs, rhs, #f"_tmp");                                  \
			if (type == llvm::Type::IntegerTyID)                                                        \
				return ctx->builder->Create##i(lhs, rhs, #i"_tmp");                                  \
			return Debugger::ErrorV( std::strcat(const_cast<char*>(Lexer::Token::Name(op)), " operation cannot apply on Non-number operands\n"),line,ch); }();
#define BITWISE(f)[&](){                                                                                                                             \
			if (type == llvm::Type::IntegerTyID)return ctx->builder->Create##f(lhs, rhs, "and_tmp");                                         \
			return Debugger::ErrorV(std::strcat(const_cast<char*>(Lexer::Token::Name(op)), " operation cannot apply on Integer operands\n"),line,ch); \
		}();

		switch (op) {


		case '+':return BASIC(FAdd, Add)
		case '-':return BASIC(FSub, Sub)
		case '*':return BASIC(FMul, Mul)
		case '%':return BASIC(FRem, SRem)
		case '<':return BASIC(FCmpULT, ICmpULT)
		case '>':return BASIC(FCmpUGT, ICmpUGT)
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
				return ctx->builder->CreateFDiv(
					lhs, rhs, "FDiv""_tmp");
			if (type == llvm::Type::IntegerTyID)
				return ctx->builder->CreateFDiv(
					ctx->builder->CreateCast(llvm::Instruction::SIToFP, lhs, llvm::Type::getDoubleTy(ctx->context)),
					ctx->builder->CreateCast(llvm::Instruction::SIToFP, rhs, llvm::Type::getDoubleTy(ctx->context)),
					"FDiv""_tmp");
			return Debugger::ErrorV(" ""'/'"" operation cannot apply on Non-number operands\n", line, ch);
		}
		case BAndAgn: { }
		case BXORAgn: { }
		case BORAgn: { }

		case '=': {
			if (lhs->getType()->getTypeID() != llvm::Type::PointerTyID)
				return Debugger::ErrorV("cannot reassign a constant\n",line,ch);
			auto rhv = rhs;
			if (rhs->getType()->getTypeID() != lhs->getType()->getPointerElementType()->getTypeID())
				rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID
				? ctx->AlignLoad(ctx->builder->CreateLoad(rhs, "rhs"))
				: rhs;
			ctx->AlignStore(ctx->builder->CreateStore(rhv, lhs));
			return lhs;
		}

#define ASSGIN(x){																											\
				auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID ? ctx->AlignLoad(ctx->builder->CreateLoad(rhs,"rhs")) : rhs;			\
				if (ltype == llvm::Type::PointerTyID)																			\
				{																										\
					type = lhs->getType()->getPointerElementType()->getTypeID();										\
					const auto lhsv = ctx->AlignLoad(ctx->builder->CreateLoad(lhs,"lhs"));															\
					return ctx->AlignStore(ctx->builder->CreateStore(x, lhs));						\
				}																										\
				return Debugger::ErrorV(" cannot reassign a constant\n",line,ch);														\
			}
#define BASIC_ASSGIN(a,b,c,d)case a:																					\
			{																											\
				auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID ? ctx->AlignLoad(ctx->builder->CreateLoad(rhs,"rhs")) : rhs;			\
				if (type == llvm::Type::PointerTyID)																			\
				{																										\
					type = lhs->getType()->getPointerElementType()->getTypeID();										\
					const auto lhsv = ctx->AlignLoad(ctx->builder->CreateLoad(lhs,"lhs"));															\
					if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)											\
						return ctx->AlignStore(ctx->builder->CreateStore(ctx->builder->Create##b(lhsv, rhv, #b"_tmp"), lhs));						\
					if (type == llvm::Type::IntegerTyID)																		\
						return ctx->AlignStore(ctx->builder->CreateStore(ctx->builder->Create##c(lhsv, rhv, #c"_tmp"), lhs));						\
					return Debugger::ErrorV(" "#d" operation cannot apply on Non-number variables\n",line,ch);							\
				}																										\
				return Debugger::ErrorV(" cannot reassign a constant\n",line,ch);														\
			}

				  // #define  BITWISE_ASSGIN(a,b,c,d)case a: {\
	              // 			if (type == Type::IntegerTyID)\
	              // 				lhs = ctx->builder->Create##c(lhs, rhs, #c"_tmp"); \
	              // 			else return ctx->LogErrorV(" "#d" operation cannot apply on Non-number variables\n");\
	              // 			return lhs;}

				  BASIC_ASSGIN(AddAgn, FAdd, Add, +=)
					  BASIC_ASSGIN(SubAgn, FSub, Sub, -=)
					  BASIC_ASSGIN(MulAgn, FMul, Mul, *=)
					  BASIC_ASSGIN(ModAgn, FRem, SRem, %=)

		case DivAgn: {
					  auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID
						  ? ctx->AlignLoad(ctx->builder->CreateLoad(rhs, "rhs"))
						  : rhs;
					  if (type == llvm::Type::PointerTyID) {
						  type = lhs->getType()->getPointerElementType()->getTypeID();
						  const auto lhsv = ctx->AlignLoad(ctx->builder->CreateLoad(lhs, "lhs"));
						  if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)
							  return ctx->AlignStore(
								  ctx->builder->CreateStore(ctx->builder->CreateFDiv(lhsv, rhv, "FDiv""_tmp"), lhs));
						  if (type == llvm::Type::IntegerTyID) {
							  const auto lhv_d = ctx->builder->CreateCast(llvm::Instruction::SIToFP, lhsv,
								  llvm::Type::getDoubleTy(ctx->context));
							  const auto rhv_d = ctx->builder->CreateCast(llvm::Instruction::SIToFP, rhv,
								  llvm::Type::getDoubleTy(ctx->context));
							  const auto div = ctx->builder->CreateFDiv(lhv_d, rhv_d, "div_tmp");
							  const auto div_i = ctx->builder->CreateCast(llvm::Instruction::FPToUI, div,
								  llvm::Type::getInt32Ty(ctx->context));
							  return ctx->AlignStore(ctx->builder->CreateStore(div_i, lhs));
						  }


						  return Debugger::ErrorV(" ""/="" operation cannot apply on Non-number variables\n",line,ch);
					  }
					  return Debugger::ErrorV(" cannot reassign a constant\n",line,ch);
				  }
		default:
			return Debugger::ErrorV("invalid binary operator",line,ch);

		}


		
	}

#define  ARGUMENTS llvm::Value* left,llvm::Value* right,std::shared_ptr<DFContext> ctx

	std::map<std::string, std::function<llvm::Value* (llvm::Value*, llvm::Value*, std::shared_ptr<DFContext>)>> Binary::gens =
	{
		{"int+int",     [](ARGUMENTS) {return ctx->builder->CreateAdd(left,right,"add_tmp");   }},
		{"int-int",     [](ARGUMENTS) {return ctx->builder->CreateSub(left,right,"sub_tmp");   }},
		{"int*int",     [](ARGUMENTS) {return ctx->builder->CreateMul(left,right,"mul_tmp");   }},
		{"int%int",     [](ARGUMENTS) {return ctx->builder->CreateSRem(left,right,"mod_tmp");   }},
		{"int/int",     [](ARGUMENTS) {return ctx->builder->CreateFDiv(
					                 ctx->builder->CreateCast(llvm::Instruction::SIToFP, left, llvm::Type::getDoubleTy(ctx->context)),
					                 ctx->builder->CreateCast(llvm::Instruction::SIToFP, right, llvm::Type::getDoubleTy(ctx->context)),
					                "div_tmp");   }},
		{"int<=int",    [](ARGUMENTS) {return ctx->builder->CreateICmpULE(left,right,"cmp_tmp");   }},
		{"int>=int",    [](ARGUMENTS) {return ctx->builder->CreateICmpUGE(left,right,"cmp_tmp");   }},
		{"int<int",     [](ARGUMENTS) {return ctx->builder->CreateICmpULT(left,right,"cmp_tmp");   }},
		{"int>int",     [](ARGUMENTS) {return ctx->builder->CreateICmpUGT(left,right,"cmp_tmp");   }},
		{"int==int",    [](ARGUMENTS) {return ctx->builder->CreateICmpEQ(left,right,"cmp_tmp");   }},
		{"int!=int",    [](ARGUMENTS) {return ctx->builder->CreateICmpNE(left,right,"cmp_tmp");   }},


		{"float+float",     [](ARGUMENTS) {return ctx->builder->CreateFAdd(left,right,"add_tmp");      }},
		{"float-float",     [](ARGUMENTS) {return ctx->builder->CreateFSub(left,right,"sub_tmp");      }},
		{"float*float",     [](ARGUMENTS) {return ctx->builder->CreateFMul(left,right,"mul_tmp");      }},
		{"float%float",     [](ARGUMENTS) {return ctx->builder->CreateFRem(left,right,"mod_tmp");   }},
		{"float/float",     [](ARGUMENTS) {return ctx->builder->CreateFDiv(left,right,"div_tmp");      }},

		{"float<=float",    [](ARGUMENTS) {return ctx->builder->CreateFCmpULE(left,right,"cmp_tmp");   }},
		{"float>=float",    [](ARGUMENTS) {return ctx->builder->CreateFCmpUGE(left,right,"cmp_tmp");   }},
		{"float<float",     [](ARGUMENTS) {return ctx->builder->CreateFCmpULT(left,right,"cmp_tmp");   }},
		{"float>float",     [](ARGUMENTS) {return ctx->builder->CreateFCmpUGT(left,right,"cmp_tmp");   }},
		{"float==float",    [](ARGUMENTS) {return ctx->builder->CreateFCmpUEQ(left,right,"cmp_tmp");   }},
		{"float!=float",    [](ARGUMENTS) {return ctx->builder->CreateFCmpUNE(left,right,"cmp_tmp");   }},
	};
#undef  ARGUMENTS
}
