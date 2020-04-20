#include "gen.h"
#include <codecvt>

llvm::LLVMContext Gen::the_context;
std::unique_ptr<llvm::Module> Gen::the_module = std::make_unique<llvm::Module>("Program", Gen::the_context);
llvm::IRBuilder<> Gen::builder(Gen::the_context);
llvm::Function* Gen::the_function = nullptr;
std::map<std::string, llvm::Value*> Gen::fields_table;
std::map<std::string, parser::ClassDecl*> Gen::types_table;
llvm::Value* Gen::True = llvm::ConstantInt::get(llvm::IntegerType::get(Gen::the_context, 1), 1);
llvm::Value* Gen::False = llvm::ConstantInt::get(llvm::IntegerType::get(Gen::the_context, 1), 0);


llvm::Value* Gen::LogErrorV(const char* str) {
    *Debugger::out << str;
    system("PAUSE");
    exit(-1);
}

llvm::GlobalVariable* Gen::CreateGlob(llvm::IRBuilder<>& builder, const std::string name, llvm::Type* ty) {
    the_module->getOrInsertGlobal(name, ty);
    auto g_var = the_module->getNamedGlobal(name);
    g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
    g_var->setAlignment(4);
    return g_var;
}

llvm::Function* Gen::CreateFunc(llvm::IRBuilder<>& builder, const std::string name) {
    const auto func_type = llvm::FunctionType::get(builder.getInt32Ty(), false);
    const auto foo_func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, name,
                                                 the_module.get());
    return foo_func;
}

llvm::BasicBlock* Gen::CreateBb(llvm::Function* func, const std::string name) {
    return llvm::BasicBlock::Create(Gen::the_context, name, func);
}

llvm::AllocaInst* Gen::CreateEntryBlockAlloca(llvm::Function* the_function, llvm::Type* type,
                                              const std::string& var_name) {
    llvm::IRBuilder<> tmp_b(&the_function->getEntryBlock(), the_function->getEntryBlock().begin());
    return tmp_b.CreateAlloca(type, 0, var_name);
}

std::string Gen::WstrToStr(const std::wstring str) {
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(str);
}

llvm::Type* Gen::GetType(std::wstring type_name) {
    auto type = 0;
    if (type_name.size() == 1)type = type_name[0];
    switch (type) {
    case '0': return llvm::Type::getVoidTy(Gen::the_context);
    case Id: return nullptr;
    case K_int: return llvm::Type::getInt32Ty(Gen::the_context);
    case K_float: return llvm::Type::getFloatTy(Gen::the_context);
    case K_double: return llvm::Type::getDoubleTy(Gen::the_context);
    case K_bool: return llvm::Type::getInt1Ty(Gen::the_context);
    case K_string: return llvm::Type::getInt8PtrTy(Gen::the_context);
    default: return the_module->getTypeByName(WstrToStr(type_name))->getPointerTo();
    }
}

llvm::StoreInst* Gen::AlignStore(llvm::StoreInst* a) {
    // a->setAlignment(MaybeAlign(8));
    return a;
}

llvm::LoadInst* Gen::AlignLoad(llvm::LoadInst* a) {
    // a->setAlignment(MaybeAlign(8));
    return a;
}

void Gen::BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg) {
    llvm::Function::Create(llvm::FunctionType::get(ret, types, isVarArg), llvm::Function::ExternalLinkage, name,
                           the_module.get());
}

namespace parser {

    void Statements::Gen() {
        stmt1->Gen();
        stmt2->Gen();
    }

    llvm::Value* NumberConst::Gen(int cmd) {
        switch (type) {
        case K_float: return llvm::ConstantFP::get(Gen::the_context, llvm::APFloat(static_cast<float>(value)));
        case K_double: return llvm::ConstantFP::get(Gen::the_context, llvm::APFloat(value));
        case K_int: return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Gen::the_context), static_cast<int>(value));
        default: return Gen::LogErrorV("Unknown number type");
        }
    }

    llvm::Value* Boolean::Gen(int cmd) {
        return value ? Gen::True : Gen::False;;
    }

    llvm::Value* String::Gen(int cmd) {
        return Gen::builder.CreateGlobalStringPtr(llvm::StringRef(Gen::WstrToStr(value)));
    }

    llvm::Value* Field::Gen(int cmd) {
        auto v = Gen::fields_table[Gen::WstrToStr(names[0])];
        if (!v) Gen::LogErrorV("Unknown variable name\n");
        std::string debug_name;
        for (const auto& name : names)debug_name += "_" + Gen::WstrToStr(name);
        if (names.size() > 1) {
            for (auto i = 1; i < names.size(); i++) {
                auto name = names[i];
                auto decl = Gen::types_table[v->getType()->getPointerElementType()->getPointerElementType()->
                                                getStructName().str()];
                auto idx = -1;
                for (int id = 0, n = decl->fields.size(); id < n; id++) {

                    if (decl->fields[id] == name) {
                        idx = id;
                        break;
                    }
                }
                if (idx == -1)return Gen::LogErrorV("Cannot find field... \n");
                v = Gen::AlignLoad(Gen::builder.CreateLoad(v, debug_name));
                v = Gen::builder.CreateStructGEP(v, idx);
            }
        }

        if (v->getType()->getTypeID() == llvm::Type::PointerTyID && cmd == 0)
            return Gen::AlignLoad(Gen::builder.CreateLoad(v, debug_name));
        return v;
    }

    llvm::Value* FuncCall::Gen(int cmd) {
        const auto callee = Gen::the_module->getFunction(Gen::WstrToStr(names[0]));
        if (!callee) return Gen::LogErrorV("Unknown function referenced");
        if (callee->arg_size() != args.size() && !callee->isVarArg())
            return Gen::LogErrorV("Incorrect # arguments passed");
        std::vector<llvm::Value*> args_v;;
        for (unsigned i = 0, e = args.size(); i != e; ++i) {
            args_v.push_back(args[i]->Gen());
            if (!args_v.back())return Gen::LogErrorV("Incorrect # arguments passed with error");
        }
        if (callee->getReturnType()->getTypeID() == llvm::Type::VoidTyID)
            return Gen::builder.CreateCall(callee, args_v);
        return Gen::builder.CreateCall(callee, args_v, "calltmp");
    }

    llvm::Value* Factor::Gen(int cmd) {
        return nullptr;
    }

    llvm::Value* Unary::Gen(int cmd) {
        const auto v = expr->Gen();
        if (!v)return nullptr;
        switch (op) {
        case '-':
        case '~':
        case '!':
        case Inc:
        case Dec:
        default:
            return Gen::LogErrorV("invalid unary operator");
        }
    }

    llvm::Value* Binary::Gen(int cmd) {
        const auto load_ptr = op == '=' || op >= AddAgn;
        auto lhs = LHS->Gen(load_ptr);
        auto rhs = RHS->Gen(load_ptr);
        if (!lhs || !rhs)return Gen::LogErrorV("  operands is NULL \n");;


        auto type = lhs->getType()->getTypeID();
        const auto ltype = lhs->getType()->getTypeID();
        const auto rtype = rhs->getType()->getTypeID();

        if (ltype != rtype) {
            if (ltype == llvm::Type::IntegerTyID) {
                if (rtype == llvm::Type::FloatTyID) {
                    type = llvm::Type::FloatTyID;
                    lhs = Gen::builder.CreateUIToFP(lhs, llvm::Type::getFloatTy(Gen::the_context));
                }
                else if (rtype == llvm::Type::DoubleTyID) {
                    type = llvm::Type::DoubleTyID;
                    lhs = Gen::builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(Gen::the_context));
                }
            }
            if (rtype == llvm::Type::IntegerTyID) {
                if (ltype == llvm::Type::FloatTyID) {
                    type = llvm::Type::FloatTyID;
                    rhs = Gen::builder.CreateUIToFP(lhs, llvm::Type::getFloatTy(Gen::the_context));
                }
                else if (ltype == llvm::Type::DoubleTyID) {
                    type = llvm::Type::DoubleTyID;
                    rhs = Gen::builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(Gen::the_context));
                }
            }
        }


        switch (op) {

#define BASIC(a,b,c)case a:{\
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)\
				return Gen::builder.Create##b(lhs, rhs, #b"_tmp");\
			if (type == llvm::Type::IntegerTyID)\
				return Gen::builder.Create##c(lhs, rhs, #c"_tmp");\
			return Gen::LogErrorV(" "#a" operation cannot apply on Non-number operands\n"); }

        BASIC('+', FAdd, Add)
        BASIC('-', FSub, Sub)
        BASIC('*', FMul, Mul)

        BASIC('%', FRem, SRem)
        case '/': {
            if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)
                return Gen::builder.CreateFDiv(
                    lhs, rhs, "FDiv""_tmp");
            if (type == llvm::Type::IntegerTyID)
                return Gen::builder.CreateFDiv(
                    Gen::builder.CreateCast(llvm::Instruction::SIToFP, lhs, llvm::Type::getDoubleTy(Gen::the_context)),
                    Gen::builder.CreateCast(llvm::Instruction::SIToFP, rhs, llvm::Type::getDoubleTy(Gen::the_context)),
                    "FDiv""_tmp");
            return Gen::LogErrorV(" ""'/'"" operation cannot apply on Non-number operands\n");
        }
        case And:
        case '&': {
            if (type == llvm::Type::IntegerTyID)return Gen::builder.CreateAnd(lhs, rhs, "and_tmp");
            return Gen::LogErrorV(" '&' operation cannot apply on Integer operands\n");
        }
        case '^': {
            if (type == llvm::Type::IntegerTyID)return Gen::builder.CreateXor(lhs, rhs, "xor_tmp");
            return Gen::LogErrorV(" '^' operation cannot apply on Integer operands\n");
        }
        case Or:
        case '|': {
            if (type == llvm::Type::IntegerTyID)return Gen::builder.CreateOr(lhs, rhs, "or_tmp");
            return Gen::LogErrorV(" '|' operation cannot apply on Integer operands\n");
        }
        case BAndAgn: { }
        case BXORAgn: { }
        case BORAgn: { }
        case Shr: {
            if (type == llvm::Type::IntegerTyID)return Gen::builder.CreateAShr(lhs, rhs, "shr_tmp");
            return Gen::LogErrorV(" '<<' operation cannot apply on Integer operands\n");
        }
        case Shl: {
            if (type == llvm::Type::IntegerTyID)return Gen::builder.CreateShl(lhs, rhs, "shl_tmp");
            return Gen::LogErrorV(" '>>' operation cannot apply on Integer operands\n");
        }

#define CMP(a,b,c)case a:\
		{\
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)\
				return Gen::builder.CreateFCmp##b(lhs, rhs, #b"_tmp");\
			if (type == llvm::Type::IntegerTyID)\
				return Gen::builder.CreateICmp##c(lhs, rhs, #b"_tmp");\
			return Gen::LogErrorV(" "#a" operation cannot apply on Non-number operands\n");\
		}
        CMP('<', ULT, ULT)
        CMP('>', UGT, UGT)
        CMP(Le, ULE, ULE)
        CMP(Ge, UGE, UGE)
        CMP(Eq, UEQ, EQ)
        CMP(Ne, UNE, NE)

        case '=': {
            if (lhs->getType()->getTypeID() != llvm::Type::PointerTyID)
                return Gen::LogErrorV(
                    "cannot reassign a constant\n");
            auto rhv = rhs;
            if (rhs->getType()->getTypeID() != lhs->getType()->getPointerElementType()->getTypeID())
                rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID
                          ? Gen::AlignLoad(Gen::builder.CreateLoad(rhs))
                          : rhs;
            Gen::AlignStore(Gen::builder.CreateStore(rhv, lhs));
            return lhs;
        }
#define BASIC_ASSGIN(a,b,c,d)case a:																					\
			{																											\
				auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID ? Gen::AlignLoad(Gen::builder.CreateLoad(rhs)) : rhs;			\
				if (type == llvm::Type::PointerTyID)																			\
				{																										\
					type = lhs->getType()->getPointerElementType()->getTypeID();										\
					const auto lhsv = Gen::AlignLoad(Gen::builder.CreateLoad(lhs));															\
					if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)											\
						return Gen::AlignStore(Gen::builder.CreateStore(Gen::builder.Create##b(lhsv, rhv, #b"_tmp"), lhs));						\
					if (type == llvm::Type::IntegerTyID)																		\
						return Gen::AlignStore(Gen::builder.CreateStore(Gen::builder.Create##c(lhsv, rhv, #c"_tmp"), lhs));						\
					return Gen::LogErrorV(" "#d" operation cannot apply on Non-number variables\n");							\
				}																										\
				return Gen::LogErrorV(" cannot reassign a constant\n");														\
			}

            // #define  BITWISE_ASSGIN(a,b,c,d)case a: {\
            // 			if (type == Type::IntegerTyID)\
            // 				lhs = Gen::builder.Create##c(lhs, rhs, #c"_tmp"); \
            // 			else return Gen::LogErrorV(" "#d" operation cannot apply on Non-number variables\n");\
            // 			return lhs;}

        BASIC_ASSGIN(AddAgn, FAdd, Add, +=)
        BASIC_ASSGIN(SubAgn, FSub, Sub, -=)
        BASIC_ASSGIN(MulAgn, FMul, Mul, *=)
        BASIC_ASSGIN(ModAgn, FRem, SRem, %=)

        case DivAgn: {
            auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID
                           ? Gen::AlignLoad(Gen::builder.CreateLoad(rhs))
                           : rhs;
            if (type == llvm::Type::PointerTyID) {
                type = lhs->getType()->getPointerElementType()->getTypeID();
                const auto lhsv = Gen::AlignLoad(Gen::builder.CreateLoad(lhs));
                if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)
                    return Gen::AlignStore(
                        Gen::builder.CreateStore(Gen::builder.CreateFDiv(lhsv, rhv, "FDiv""_tmp"), lhs));
                if (type == llvm::Type::IntegerTyID) {
                    const auto lhv_d = Gen::builder.CreateCast(llvm::Instruction::SIToFP, lhsv,
                                                               llvm::Type::getDoubleTy(Gen::the_context));
                    const auto rhv_d = Gen::builder.CreateCast(llvm::Instruction::SIToFP, rhv,
                                                               llvm::Type::getDoubleTy(Gen::the_context));
                    const auto div = Gen::builder.CreateFDiv(lhv_d, rhv_d, "div_tmp");
                    const auto div_i = Gen::builder.CreateCast(llvm::Instruction::FPToUI, div,
                                                               llvm::Type::getInt32Ty(Gen::the_context));
                    return Gen::AlignStore(Gen::builder.CreateStore(div_i, lhs));
                }


                return Gen::LogErrorV(" ""/="" operation cannot apply on Non-number variables\n");
            }
            return Gen::LogErrorV(" cannot reassign a constant\n");
        }
        default:
            return Gen::LogErrorV("invalid binary operator");

        }
    }

    llvm::Value* Ternary::Gen(int cmd) {
        return nullptr;
    }

    void FunctionDecl::GenHeader() {
        auto the_function = Gen::the_module->getFunction(Gen::WstrToStr(name));

        if (!the_function) {
            std::vector<llvm::Type*> types;
            if (self_type != nullptr) {
                types.push_back(self_type->getPointerTo());
                args->names.insert(args->names.begin(), L"this");
            }
            for (auto i = 0; i < args->size; i++)
                types.push_back(Gen::GetType(args->types[i]));

            const auto func_type = llvm::FunctionType::get(Gen::GetType(return_type), types, args->isVarArg);
            the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, Gen::WstrToStr(name),
                                                  Gen::the_module.get());

            unsigned idx = 0;
            for (auto& arg : the_function->args())
                arg.setName(Gen::WstrToStr(args->names[idx++]));

        }
        else *Debugger::out << "function " << name << " already defined";
    }

    void FunctionDecl::Gen() {
        if (is_extern)return;
        auto function = Gen::the_module->getFunction(Gen::WstrToStr(name));
        if (!function) {
            *Debugger::out << "function head not found\n";
            return;
        }
        const auto bb = llvm::BasicBlock::Create(Gen::the_context, Gen::WstrToStr(name) + "_entry", function);
        Gen::builder.SetInsertPoint(bb);


        Gen::fields_table.clear();
        for (auto& arg : function->args())Gen::fields_table[arg.getName()] = &arg;

        Gen::the_function = function;
        if (statements != nullptr)statements->Gen();

        verifyFunction(*function);
        return;
    }

    void FieldDecl::Gen() {
        const auto _name = Gen::WstrToStr(name);
        const auto val = value->Gen();

        auto ty = type.size() == 0 ? val->getType() : Gen::GetType(type);
        if (!val) return;

        if (constant) {
            const auto const_v = static_cast<llvm::ConstantFP*>(val);
            const auto v = Gen::CreateGlob(Gen::builder, _name, Gen::builder.getDoubleTy());
            v->setInitializer(const_v);
            Gen::fields_table[_name] = v;
        }
        else {
            const auto alloca = Gen::CreateEntryBlockAlloca(Gen::the_function, ty, _name);
            alloca->setAlignment(llvm::MaybeAlign(8));

            Gen::AlignStore(Gen::builder.CreateStore(val, alloca));
            Gen::fields_table[_name] = alloca;
        }

    }

    void ClassDecl::GenHeader() {
        auto the_struct = Gen::the_module->getTypeByName(Gen::WstrToStr(name));
        if (!the_struct) the_struct = llvm::StructType::create(Gen::the_context, Gen::WstrToStr(name));
        else *Debugger::out << "Type " << name << " already defined" << std::endl;
    }

    void ClassDecl::Gen() {
        auto the_struct = Gen::the_module->getTypeByName(Gen::WstrToStr(name));
        std::vector<llvm::Type*> field_tys;
        for (const auto& type : types)field_tys.push_back(Gen::GetType(type));
        the_struct->setBody(field_tys);
        Gen::types_table[the_struct->getName().str()] = this;

        //Create a Constructor function
        const auto func_type = llvm::FunctionType::get(the_struct->getPointerTo(), std::vector<llvm::Type*>(), false);
        auto function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, Gen::WstrToStr(name),
                                               Gen::the_module.get());
        const auto bb = llvm::BasicBlock::Create(Gen::the_context, Gen::WstrToStr(name) + "_entry", function);
        function->setCallingConv(llvm::CallingConv::C);
        Gen::builder.SetInsertPoint(bb);
        //Constructor Body

        // const auto alloca = CreateEntryBlockAlloca(the_function, the_struct, "struct");
        // alloca->setAlignment(MaybeAlign(8));

        const auto ptr = Gen::builder.CreateCall(Gen::the_module->getFunction("malloc"),
                                                 llvm::ConstantInt::get(llvm::Type::getInt32Ty(Gen::the_context), 32));
        auto p = Gen::builder.CreateCast(llvm::Instruction::BitCast, ptr, the_struct->getPointerTo());

        Gen::builder.CreateRet(p);
        // auto field1= Gen::builder.CreateStructGEP(the_struct, alloca, 1);
        // Gen::builder.CreateStore(llvm::ConstantInt::get(Type::getInt32Ty(Gen::the_context), 233),field1);
        verifyFunction(*function);

        for (auto& function : functions) {
            function->SetInternal(name, the_struct);
            function->GenHeader();
            function->Gen();
        }
    }

    void If::Gen() {
        auto cond_v = condition->Gen();
        if (!cond_v) {
            Debugger::AlertNonBreak(L"Error in condititon");
            return;
        }

        cond_v = Gen::builder.CreateICmpEQ(cond_v, Gen::True, "ifcond");
        auto function = Gen::builder.GetInsertBlock()->getParent();

        auto then_bb = llvm::BasicBlock::Create(Gen::the_context, "then", function);
        auto else_bb = llvm::BasicBlock::Create(Gen::the_context, "else");
        const auto merge_bb = llvm::BasicBlock::Create(Gen::the_context, "ifcont");

        Gen::builder.CreateCondBr(cond_v, then_bb, else_bb);

        Gen::builder.SetInsertPoint(then_bb);


        stmts->Gen();


        Gen::builder.CreateBr(merge_bb);
        // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
        then_bb = Gen::builder.GetInsertBlock();

        // Emit else block.
        function->getBasicBlockList().push_back(else_bb);
        Gen::builder.SetInsertPoint(else_bb);

        else_stmts->Gen();


        Gen::builder.CreateBr(merge_bb);
        // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
        else_bb = Gen::builder.GetInsertBlock();

        // Emit merge block.
        function->getBasicBlockList().push_back(merge_bb);
        Gen::builder.SetInsertPoint(merge_bb);

        // PHINode* PN = Gen::builder.CreatePHI(Type::getDoubleTy(Gen::the_context), 2, "iftmp");
        // PN->addIncoming(ThenV, ThenBB);
        // PN->addIncoming(ElseV, ElseBB);
        return;

    }

    void Empty::Gen() {
        value->Gen();
    }

    void Throw::Gen() { }

    void Return::Gen() {
        if (value == nullptr) {
            Gen::builder.CreateRetVoid();
            return;
        }
        const auto val = value->Gen();
        if (!val) Gen::LogErrorV("Error in return");
        else Gen::builder.CreateRet(val);
    }

    void Break::Gen() { }

    void Continue::Gen() { }

    void Import::Gen() { }

    void While::Gen() { }

    void Do::Gen() { }

    void For::Gen() { }

    void Program::Gen() {
        Gen::BuildInFunc("malloc", llvm::Type::getInt8PtrTy(Gen::the_context),
                         std::vector<llvm::Type*>{llvm::Type::getInt32Ty(Gen::the_context)});
        Gen::BuildInFunc("free", llvm::Type::getVoidTy(Gen::the_context),
                         std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(Gen::the_context)});
        Gen::BuildInFunc("printf", llvm::Type::getVoidTy(Gen::the_context),
                         std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(Gen::the_context)}, true);

        for (auto& declaration : declarations)declaration->GenHeader();
        for (auto& declaration : declarations)declaration->Gen();

        const auto main_func = Gen::CreateFunc(Gen::builder, "main");
        const auto entry = Gen::CreateBb(main_func, "entry");
        Gen::the_function = main_func;
        Gen::builder.SetInsertPoint(entry);

        for (auto& statement : statements)if (statement != nullptr)statement->Gen();

        Gen::builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(Gen::the_context), 0));
        verifyFunction(*main_func);
    }

}
