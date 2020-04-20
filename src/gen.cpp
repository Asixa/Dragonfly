#include "gen.h"


llvm::LLVMContext gen::the_context;
std::unique_ptr<llvm::Module> gen::the_module = std::make_unique<llvm::Module>("Program", gen::the_context);
llvm::IRBuilder<> gen::builder(gen::the_context);
llvm::Function* gen::the_function = nullptr;
std::map<std::string, llvm::Value*> gen::named_values;
std::map<std::string, parser::ClassDecl*> gen::named_types;
llvm::Value* gen::True = llvm::ConstantInt::get(llvm::IntegerType::get(gen::the_context, 1), 1);
llvm::Value* gen::False = llvm::ConstantInt::get(llvm::IntegerType::get(gen::the_context, 1), 0);


llvm::Value* gen::LogErrorV(const char* str) {
    *debugger::out << str;
    system("PAUSE");
    exit(-1);
}

llvm::GlobalVariable* gen::CreateGlob(llvm::IRBuilder<>& builder, const std::string name, llvm::Type* ty) {
    the_module->getOrInsertGlobal(name, ty);
    auto g_var = the_module->getNamedGlobal(name);
    g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
    g_var->setAlignment(4);
    return g_var;
}

llvm::Function* gen::CreateFunc(llvm::IRBuilder<>& builder, const std::string name) {
    const auto func_type = llvm::FunctionType::get(builder.getInt32Ty(), false);
    const auto foo_func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, name,
                                                 the_module.get());
    return foo_func;
}

llvm::BasicBlock* gen::CreateBb(llvm::Function* func, const std::string name) {
    return llvm::BasicBlock::Create(gen::the_context, name, func);
}

llvm::AllocaInst* gen::CreateEntryBlockAlloca(llvm::Function* the_function, llvm::Type* type,
                                              const std::string& var_name) {
    llvm::IRBuilder<> tmp_b(&the_function->getEntryBlock(), the_function->getEntryBlock().begin());
    return tmp_b.CreateAlloca(type, 0, var_name);
}

std::string gen::WstrToStr(const std::wstring str) {
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(str);
}

llvm::Type* gen::GetType(std::wstring type_name) {
    auto type = 0;
    if (type_name.size() == 1)type = type_name[0];
    switch (type) {
    case '0': return llvm::Type::getVoidTy(gen::the_context);
    case Id: return nullptr;
    case K_int: return llvm::Type::getInt32Ty(gen::the_context);
    case K_float: return llvm::Type::getFloatTy(gen::the_context);
    case K_double: return llvm::Type::getDoubleTy(gen::the_context);
    case K_bool: return llvm::Type::getInt1Ty(gen::the_context);
    case K_string: return llvm::Type::getInt8PtrTy(gen::the_context);
    default: return the_module->getTypeByName(WstrToStr(type_name))->getPointerTo();
    }
}

llvm::StoreInst* gen::AlignStore(llvm::StoreInst* a) {
    // a->setAlignment(MaybeAlign(8));
    return a;
}

llvm::LoadInst* gen::AlignLoad(llvm::LoadInst* a) {
    // a->setAlignment(MaybeAlign(8));
    return a;
}

void gen::BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg) {
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
        case K_float: return llvm::ConstantFP::get(gen::the_context, llvm::APFloat(static_cast<float>(value)));
        case K_double: return llvm::ConstantFP::get(gen::the_context, llvm::APFloat(value));
        case K_int: return llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen::the_context), static_cast<int>(value));
        default: return gen::LogErrorV("Unknown number type");
        }
    }

    llvm::Value* Boolean::Gen(int cmd) {
        return value ? gen::True : gen::False;
        // const auto bool_type=llvm::IntegerType::get(gen::the_context, 1);
        // return llvm::ConstantInt::get(bool_type, value);
    }

    llvm::Value* String::Gen(int cmd) {
        return gen::builder.CreateGlobalStringPtr(llvm::StringRef(gen::WstrToStr(value)));
        // return ConstantDataArray::getString(gen::the_context, WstrToStr(value), true);
    }

    llvm::Value* Field::Gen(int cmd) {
        auto v = gen::named_values[gen::WstrToStr(names[0])];
        if (!v) gen::LogErrorV("Unknown variable name\n");
        std::string debugname;
        for (const auto& name : names)debugname += "_" + gen::WstrToStr(name);
        if (names.size() > 1) {
            for (auto i = 1; i < names.size(); i++) {
                auto name = names[i];
                auto decl = gen::named_types[v->getType()->getPointerElementType()->getPointerElementType()->
                                                getStructName().str()];
                auto idx = -1;
                for (int id = 0, n = decl->fields.size(); id < n; id++) {

                    if (decl->fields[id] == name) {
                        idx = id;
                        break;
                    }
                }
                if (idx == -1)return gen::LogErrorV("Cannot find field... \n");
                v = gen::AlignLoad(gen::builder.CreateLoad(v, debugname));
                v = gen::builder.CreateStructGEP(v, idx);
            }
        }

        if (v->getType()->getTypeID() == llvm::Type::PointerTyID && cmd == 0)
            return gen::AlignLoad(gen::builder.CreateLoad(v, debugname));
        return v;
    }

    llvm::Value* FuncCall::Gen(int cmd) {
        const auto callee = gen::the_module->getFunction(gen::WstrToStr(names[0]));
        if (!callee) return gen::LogErrorV("Unknown function referenced");
        if (callee->arg_size() != args.size() && !callee->isVarArg())
            return gen::LogErrorV("Incorrect # arguments passed");
        std::vector<llvm::Value*> args_v;;
        for (unsigned i = 0, e = args.size(); i != e; ++i) {
            args_v.push_back(args[i]->Gen());
            if (!args_v.back())return gen::LogErrorV("Incorrect # arguments passed with error");
        }
        if (callee->getReturnType()->getTypeID() == llvm::Type::VoidTyID)
            return gen::builder.CreateCall(callee, args_v);
        return gen::builder.CreateCall(callee, args_v, "calltmp");
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
            return gen::LogErrorV("invalid binary operator");

        }
    }

    llvm::Value* Binary::Gen(int cmd) {
        const auto load_ptr = op == '=' || op >= AddAgn;
        auto lhs = LHS->Gen(load_ptr);
        auto rhs = RHS->Gen(load_ptr);
        if (!lhs || !rhs)return gen::LogErrorV("  operands is NULL \n");;


        auto type = lhs->getType()->getTypeID();
        const auto ltype = lhs->getType()->getTypeID();
        const auto rtype = rhs->getType()->getTypeID();

        if (ltype != rtype) {
            if (ltype == llvm::Type::IntegerTyID) {
                if (rtype == llvm::Type::FloatTyID) {
                    type = llvm::Type::FloatTyID;
                    lhs = gen::builder.CreateUIToFP(lhs, llvm::Type::getFloatTy(gen::the_context));
                }
                else if (rtype == llvm::Type::DoubleTyID) {
                    type = llvm::Type::DoubleTyID;
                    lhs = gen::builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(gen::the_context));
                }
            }
            if (rtype == llvm::Type::IntegerTyID) {
                if (ltype == llvm::Type::FloatTyID) {
                    type = llvm::Type::FloatTyID;
                    rhs = gen::builder.CreateUIToFP(lhs, llvm::Type::getFloatTy(gen::the_context));
                }
                else if (ltype == llvm::Type::DoubleTyID) {
                    type = llvm::Type::DoubleTyID;
                    rhs = gen::builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(gen::the_context));
                }
            }
        }


        switch (op) {

#define BASIC(a,b,c)case a:{\
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)\
				return gen::builder.Create##b(lhs, rhs, #b"_tmp");\
			if (type == llvm::Type::IntegerTyID)\
				return gen::builder.Create##c(lhs, rhs, #c"_tmp");\
			return gen::LogErrorV(" "#a" operation cannot apply on Non-number operands\n"); }

        BASIC('+', FAdd, Add)
        BASIC('-', FSub, Sub)
        BASIC('*', FMul, Mul)

        BASIC('%', FRem, SRem)
        case '/': {
            if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)
                return gen::builder.CreateFDiv(
                    lhs, rhs, "FDiv""_tmp");
            if (type == llvm::Type::IntegerTyID)
                return gen::builder.CreateFDiv(
                    gen::builder.CreateCast(llvm::Instruction::SIToFP, lhs, llvm::Type::getDoubleTy(gen::the_context)),
                    gen::builder.CreateCast(llvm::Instruction::SIToFP, rhs, llvm::Type::getDoubleTy(gen::the_context)),
                    "FDiv""_tmp");
            return gen::LogErrorV(" ""'/'"" operation cannot apply on Non-number operands\n");
        }
        case And:
        case '&': {
            if (type == llvm::Type::IntegerTyID)return gen::builder.CreateAnd(lhs, rhs, "and_tmp");
            return gen::LogErrorV(" '&' operation cannot apply on Integer operands\n");
        }
        case '^': {
            if (type == llvm::Type::IntegerTyID)return gen::builder.CreateXor(lhs, rhs, "xor_tmp");
            return gen::LogErrorV(" '^' operation cannot apply on Integer operands\n");
        }
        case Or:
        case '|': {
            if (type == llvm::Type::IntegerTyID)return gen::builder.CreateOr(lhs, rhs, "or_tmp");
            return gen::LogErrorV(" '|' operation cannot apply on Integer operands\n");
        }
        case BAndAgn: { }
        case BXORAgn: { }
        case BORAgn: { }
        case Shr: {
            if (type == llvm::Type::IntegerTyID)return gen::builder.CreateAShr(lhs, rhs, "shr_tmp");
            return gen::LogErrorV(" '<<' operation cannot apply on Integer operands\n");
        }
        case Shl: {
            if (type == llvm::Type::IntegerTyID)return gen::builder.CreateShl(lhs, rhs, "shl_tmp");
            return gen::LogErrorV(" '>>' operation cannot apply on Integer operands\n");
        }

#define CMP(a,b,c)case a:\
		{\
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)\
				return gen::builder.CreateFCmp##b(lhs, rhs, #b"_tmp");\
			if (type == llvm::Type::IntegerTyID)\
				return gen::builder.CreateICmp##c(lhs, rhs, #b"_tmp");\
			return gen::LogErrorV(" "#a" operation cannot apply on Non-number operands\n");\
		}
        CMP('<', ULT, ULT)
        CMP('>', UGT, UGT)
        CMP(Le, ULE, ULE)
        CMP(Ge, UGE, UGE)
        CMP(Eq, UEQ, EQ)
        CMP(Ne, UNE, NE)

        case '=': {
            if (lhs->getType()->getTypeID() != llvm::Type::PointerTyID)
                return gen::LogErrorV(
                    "cannot reassign a constant\n");
            auto rhv = rhs;
            if (rhs->getType()->getTypeID() != lhs->getType()->getPointerElementType()->getTypeID())
                rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID
                          ? gen::AlignLoad(gen::builder.CreateLoad(rhs))
                          : rhs;
            gen::AlignStore(gen::builder.CreateStore(rhv, lhs));
            return lhs;
        }
#define BASIC_ASSGIN(a,b,c,d)case a:																					\
			{																											\
				auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID ? gen::AlignLoad(gen::builder.CreateLoad(rhs)) : rhs;			\
				if (type == llvm::Type::PointerTyID)																			\
				{																										\
					type = lhs->getType()->getPointerElementType()->getTypeID();										\
					const auto lhsv = gen::AlignLoad(gen::builder.CreateLoad(lhs));															\
					if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)											\
						return gen::AlignStore(gen::builder.CreateStore(gen::builder.Create##b(lhsv, rhv, #b"_tmp"), lhs));						\
					if (type == llvm::Type::IntegerTyID)																		\
						return gen::AlignStore(gen::builder.CreateStore(gen::builder.Create##c(lhsv, rhv, #c"_tmp"), lhs));						\
					return gen::LogErrorV(" "#d" operation cannot apply on Non-number variables\n");							\
				}																										\
				return gen::LogErrorV(" cannot reassign a constant\n");														\
			}

            // #define  BITWISE_ASSGIN(a,b,c,d)case a: {\
            // 			if (type == Type::IntegerTyID)\
            // 				lhs = gen::builder.Create##c(lhs, rhs, #c"_tmp"); \
            // 			else return gen::LogErrorV(" "#d" operation cannot apply on Non-number variables\n");\
            // 			return lhs;}

        BASIC_ASSGIN(AddAgn, FAdd, Add, +=)
        BASIC_ASSGIN(SubAgn, FSub, Sub, -=)
        BASIC_ASSGIN(MulAgn, FMul, Mul, *=)
        BASIC_ASSGIN(ModAgn, FRem, SRem, %=)

        case DivAgn: {
            auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID
                           ? gen::AlignLoad(gen::builder.CreateLoad(rhs))
                           : rhs;
            if (type == llvm::Type::PointerTyID) {
                type = lhs->getType()->getPointerElementType()->getTypeID();
                const auto lhsv = gen::AlignLoad(gen::builder.CreateLoad(lhs));
                if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)
                    return gen::AlignStore(
                        gen::builder.CreateStore(gen::builder.CreateFDiv(lhsv, rhv, "FDiv""_tmp"), lhs));
                if (type == llvm::Type::IntegerTyID) {
                    const auto lhv_d = gen::builder.CreateCast(llvm::Instruction::SIToFP, lhsv,
                                                               llvm::Type::getDoubleTy(gen::the_context));
                    const auto rhv_d = gen::builder.CreateCast(llvm::Instruction::SIToFP, rhv,
                                                               llvm::Type::getDoubleTy(gen::the_context));
                    const auto div = gen::builder.CreateFDiv(lhv_d, rhv_d, "div_tmp");
                    const auto div_i = gen::builder.CreateCast(llvm::Instruction::FPToUI, div,
                                                               llvm::Type::getInt32Ty(gen::the_context));
                    return gen::AlignStore(gen::builder.CreateStore(div_i, lhs));
                }


                return gen::LogErrorV(" ""/="" operation cannot apply on Non-number variables\n");
            }
            return gen::LogErrorV(" cannot reassign a constant\n");
        }
        default:
            return gen::LogErrorV("invalid binary operator");

        }
    }

    llvm::Value* Ternary::Gen(int cmd) {
        return nullptr;
    }

    void FunctionDecl::GenHeader() {
        auto the_function = gen::the_module->getFunction(gen::WstrToStr(name));

        if (!the_function) {
            std::vector<llvm::Type*> types;
            if (self_type != nullptr) {
                types.push_back(self_type->getPointerTo());
                args->names.insert(args->names.begin(), L"this");
            }
            for (auto i = 0; i < args->size; i++)
                types.push_back(gen::GetType(args->types[i]));

            const auto func_type = llvm::FunctionType::get(gen::GetType(return_type), types, args->isVarArg);
            the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, gen::WstrToStr(name),
                                                  gen::the_module.get());

            unsigned idx = 0;
            for (auto& arg : the_function->args())
                arg.setName(gen::WstrToStr(args->names[idx++]));

        }
        else *debugger::out << "function " << name << " already defined";
    }

    void FunctionDecl::Gen() {
        if (is_extern)return;
        auto function = gen::the_module->getFunction(gen::WstrToStr(name));
        if (!function) {
            *debugger::out << "function head not found\n";
            return;
        }
        const auto bb = llvm::BasicBlock::Create(gen::the_context, gen::WstrToStr(name) + "_entry", function);
        gen::builder.SetInsertPoint(bb);


        gen::named_values.clear();
        for (auto& arg : function->args())gen::named_values[arg.getName()] = &arg;

        gen::the_function = function;
        if (statements != nullptr)statements->Gen();

        verifyFunction(*function);
        return;
    }

    void FieldDecl::Gen() {
        const auto _name = gen::WstrToStr(name);
        const auto val = value->Gen();

        auto ty = type.size() == 0 ? val->getType() : gen::GetType(type);
        if (!val) return;

        if (constant) {
            const auto const_v = static_cast<llvm::ConstantFP*>(val);
            const auto v = gen::CreateGlob(gen::builder, _name, gen::builder.getDoubleTy());
            v->setInitializer(const_v);
            gen::named_values[_name] = v;
        }
        else {
            const auto alloca = gen::CreateEntryBlockAlloca(gen::the_function, ty, _name);
            alloca->setAlignment(llvm::MaybeAlign(8));

            gen::AlignStore(gen::builder.CreateStore(val, alloca));
            gen::named_values[_name] = alloca;
        }

    }

    void ClassDecl::GenHeader() {
        auto the_struct = gen::the_module->getTypeByName(gen::WstrToStr(name));
        if (!the_struct) the_struct = llvm::StructType::create(gen::the_context, gen::WstrToStr(name));
        else *debugger::out << "Type " << name << " already defined" << std::endl;
    }

    void ClassDecl::Gen() {
        auto the_struct = gen::the_module->getTypeByName(gen::WstrToStr(name));
        std::vector<llvm::Type*> field_tys;
        for (const auto& type : types)field_tys.push_back(gen::GetType(type));
        the_struct->setBody(field_tys);
        gen::named_types[the_struct->getName().str()] = this;

        //Create a Constructor function
        const auto func_type = llvm::FunctionType::get(the_struct->getPointerTo(), std::vector<llvm::Type*>(), false);
        auto function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, gen::WstrToStr(name),
                                               gen::the_module.get());
        const auto bb = llvm::BasicBlock::Create(gen::the_context, gen::WstrToStr(name) + "_entry", function);
        function->setCallingConv(llvm::CallingConv::C);
        gen::builder.SetInsertPoint(bb);
        //Constructor Body

        // const auto alloca = CreateEntryBlockAlloca(the_function, the_struct, "struct");
        // alloca->setAlignment(MaybeAlign(8));

        const auto ptr = gen::builder.CreateCall(gen::the_module->getFunction("malloc"),
                                                 llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen::the_context), 32));
        auto p = gen::builder.CreateCast(llvm::Instruction::BitCast, ptr, the_struct->getPointerTo());

        gen::builder.CreateRet(p);
        // auto field1= gen::builder.CreateStructGEP(the_struct, alloca, 1);
        // gen::builder.CreateStore(llvm::ConstantInt::get(Type::getInt32Ty(gen::the_context), 233),field1);
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
            debugger::AlertNonBreak(L"Error in condititon");
            return;
        }

        cond_v = gen::builder.CreateICmpEQ(cond_v, gen::True, "ifcond");
        auto function = gen::builder.GetInsertBlock()->getParent();

        auto then_bb = llvm::BasicBlock::Create(gen::the_context, "then", function);
        auto else_bb = llvm::BasicBlock::Create(gen::the_context, "else");
        const auto merge_bb = llvm::BasicBlock::Create(gen::the_context, "ifcont");

        gen::builder.CreateCondBr(cond_v, then_bb, else_bb);

        gen::builder.SetInsertPoint(then_bb);


        stmts->Gen();


        gen::builder.CreateBr(merge_bb);
        // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
        then_bb = gen::builder.GetInsertBlock();

        // Emit else block.
        function->getBasicBlockList().push_back(else_bb);
        gen::builder.SetInsertPoint(else_bb);

        else_stmts->Gen();


        gen::builder.CreateBr(merge_bb);
        // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
        else_bb = gen::builder.GetInsertBlock();

        // Emit merge block.
        function->getBasicBlockList().push_back(merge_bb);
        gen::builder.SetInsertPoint(merge_bb);

        // PHINode* PN = gen::builder.CreatePHI(Type::getDoubleTy(gen::the_context), 2, "iftmp");
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
            gen::builder.CreateRetVoid();
            return;
        }
        const auto val = value->Gen();
        if (!val) gen::LogErrorV("Error in return");
        else gen::builder.CreateRet(val);
    }

    void Break::Gen() { }

    void Continue::Gen() { }

    void Import::Gen() { }

    void While::Gen() { }

    void Do::Gen() { }

    void For::Gen() { }

    void Program::Gen() {
        gen::BuildInFunc("malloc", llvm::Type::getInt8PtrTy(gen::the_context),
                         std::vector<llvm::Type*>{llvm::Type::getInt32Ty(gen::the_context)});
        gen::BuildInFunc("free", llvm::Type::getVoidTy(gen::the_context),
                         std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(gen::the_context)});
        gen::BuildInFunc("printf", llvm::Type::getVoidTy(gen::the_context),
                         std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(gen::the_context)}, true);

        for (auto& declaration : declarations)declaration->GenHeader();
        for (auto& declaration : declarations)declaration->Gen();

        const auto main_func = gen::CreateFunc(gen::builder, "main");
        const auto entry = gen::CreateBb(main_func, "entry");
        gen::the_function = main_func;
        gen::builder.SetInsertPoint(entry);

        for (auto& statement : statements)if (statement != nullptr)statement->Gen();

        gen::builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen::the_context), 0));
        verifyFunction(*main_func);
    }

}
