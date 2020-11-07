#include "AST/expressions/binary.h"
#include "LLVM/context.h"
#include "AST/declarations/function-decl.h"
namespace AST {
	using namespace expr;
	void Binary::ToString() {
		*Debugger::out << "(";
		LHS->ToString();
		*Debugger::out << " " << Lexer::Token::Name(op) << " ";
		RHS->ToString();
		*Debugger::out << ")";
	}

	std::shared_ptr<AST::Type> Binary::Analysis(std::shared_ptr<DFContext>ctx) {
		auto op_name = std::string(Lexer::Token::Name(op));
		assign = op == '=' || op >= AddAgn;  // check if it is += -= /= *= ...
		if (assign) op_name.pop_back();
		const auto lhs_type = LHS->Analysis(ctx), rhs_type = RHS->Analysis(ctx);
		if (assign) { return lhs_type; }
        if(lhs_type->category==Type::Basic && rhs_type->category == Type::Basic) {
            const auto lhs_basic = std::static_pointer_cast<BasicType>(lhs_type);
            const auto rhs_basic =std::static_pointer_cast<BasicType>(rhs_type);
			func_name = lhs_type->ToString() + op_name + rhs_type->ToString();
			if (lhs_basic->detail == rhs_basic->detail)return lhs_type;
			auto max = std::max(lhs_basic->detail, rhs_basic->detail);
			auto max_type = std::make_shared<BasicType>(max);
			targetTy = max_type->ToLLVM(ctx)->getTypeID();
			func_name = max_type->ToString() + op_name + max_type->ToString();
			Debugger::Debug("binary func:{} , target type:{}", func_name , targetTy);
			if (gens.find(func_name) != gens.end())return max_type;
            Debugger::Error("{} operation is not defined between {} and {}", op_name, lhs_type->ToString(), rhs_type->ToString());
            return nullptr;
        }
		func_name = lhs_type->ToString()+ op_name + rhs_type->ToString();
        const auto func=ctx->ast->GetFunctionDecl(func_name);
		if (func)return func->return_type;
	    Debugger::ErrorV(line,ch,"binary operation '{}' between '{}' and '{}' is not found", op_name, lhs_type->ToString(), rhs_type->ToString());
        return nullptr;
    }


	llvm::Value* Conv(std::shared_ptr<DFContext> ctx,llvm::Value* v,llvm::Type::TypeID ty) {
        if(v->getType()->getTypeID()== llvm::Type::IntegerTyID && v->getType()->getIntegerBitWidth()<32) 
			v = ctx->builder->CreateCast(llvm::Instruction::SExt, v, llvm::Type::getInt32Ty(ctx->context));
	    if(ty==llvm::Type::IntegerTyID)return v;
		v = ctx->builder->CreateCast(llvm::Instruction::SIToFP, v, llvm::Type::getFloatTy(ctx->context));
		if (ty == llvm::Type::FloatTyID)return v;
		v = ctx->builder->CreateCast(llvm::Instruction::SExt, v, llvm::Type::getDoubleTy(ctx->context));
		auto t=llvm::Type::getInt8Ty(ctx->context);
		return v;
	}

	llvm::Value* Binary::Gen(std::shared_ptr<DFContext> ctx, bool is_ptr) {
		const auto lhs_raw = LHS->Gen(ctx, assign);
        const auto lhs = assign?ctx->builder->CreateLoad(lhs_raw):lhs_raw,rhs = RHS->Gen(ctx);
		const auto lhv = targetTy == 0 ? lhs:  Conv(ctx, lhs, targetTy);
        const auto rhv = targetTy == 0 ? rhs : Conv(ctx, rhs, targetTy);
		llvm::Value* result = nullptr;
        if(op=='=') result=rhs;
        else result = gens.find(func_name) != gens.end()
                                  ? gens[func_name](lhv, rhv, ctx)
                                  : ctx->builder->CreateCall(ctx->llvm->GetFunction(func_name), {lhv, rhv});
		if (assign) ctx->builder->CreateStore(result, lhs_raw);
			// if (!is_ptr) result = ctx->builder->CreateLoad(lhs_raw);
		return result;
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
		{"int<=int",    [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt, ctx->builder->CreateICmpULE(left,right,"cmp_tmp"),ctx->constant.int8);     }},
		{"int>=int",    [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt, ctx->builder->CreateICmpUGE(left,right,"cmp_tmp"),ctx->constant.int8);     }},
		{"int<int",     [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt, ctx->builder->CreateICmpULT(left,right,"cmp_tmp"),ctx->constant.int8);     }},
		{"int>int",     [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt, ctx->builder->CreateICmpUGT(left,right,"cmp_tmp"),ctx->constant.int8);     }},
		{"int==int",    [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt, ctx->builder->CreateICmpEQ(left,right,"cmp_tmp"), ctx->constant.int8);     }},
		{"int!=int",    [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt, ctx->builder->CreateICmpNE(left,right,"cmp_tmp"), ctx->constant.int8);     }},

		{"float+float",     [](ARGUMENTS) {return ctx->builder->CreateFAdd(left,right,"add_tmp");      }},
		{"float-float",     [](ARGUMENTS) {return ctx->builder->CreateFSub(left,right,"sub_tmp");      }},
		{"float*float",     [](ARGUMENTS) {return ctx->builder->CreateFMul(left,right,"mul_tmp");      }},
		{"float%float",     [](ARGUMENTS) {return ctx->builder->CreateFRem(left,right,"mod_tmp");      }},
		{"float/float",     [](ARGUMENTS) {return ctx->builder->CreateFDiv(left,right,"div_tmp");      }},

		{"float<=float",    [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt,ctx->builder->CreateFCmpULE(left,right,"cmp_tmp"), ctx->constant.int8);    }},
		{"float>=float",    [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt,ctx->builder->CreateFCmpUGE(left,right,"cmp_tmp"), ctx->constant.int8);   }},
		{"float<float",     [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt,ctx->builder->CreateFCmpULT(left,right,"cmp_tmp"), ctx->constant.int8);   }},
		{"float>float",     [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt,ctx->builder->CreateFCmpUGT(left,right,"cmp_tmp"), ctx->constant.int8);    }},
		{"float==float",    [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt,ctx->builder->CreateFCmpUEQ(left,right,"cmp_tmp"), ctx->constant.int8);    }},
		{"float!=float",    [](ARGUMENTS) {return ctx->builder->CreateCast(llvm::Instruction::ZExt,ctx->builder->CreateFCmpUNE(left,right,"cmp_tmp"), ctx->constant.int8);    }},
	};
#undef  ARGUMENTS
}
