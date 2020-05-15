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


#include <codecvt>
#include <iostream>
#include <fstream>

#include <llvm/Support/raw_ostream.h>

#include "codegen.h"
llvm::LLVMContext CodeGen::the_context;
std::unique_ptr<llvm::Module> CodeGen::the_module = std::make_unique<llvm::Module>("Program", CodeGen::the_context);
llvm::IRBuilder<> CodeGen::builder(CodeGen::the_context);
// llvm::Function* CodeGen::the_function = nullptr;
std::map<std::string, llvm::Value*> CodeGen::local_fields_table;
std::map<std::string, llvm::Value*> CodeGen::global_fields_table;
std::map<std::string, parser::ClassDecl*> CodeGen::types_table;
llvm::Value* CodeGen::True = llvm::ConstantInt::get(llvm::IntegerType::get(CodeGen::the_context, 1), 1);
llvm::Value* CodeGen::False = llvm::ConstantInt::get(llvm::IntegerType::get(CodeGen::the_context, 1), 0);

bool CodeGen::is_sub_block = false;
llvm::BasicBlock* CodeGen::block_begin = nullptr;
llvm::BasicBlock* CodeGen::block_end = nullptr;

// llvm::Value* CodeGen::This = nullptr;

llvm::Value* CodeGen::LogErrorV(const char* str) {
    *Debugger::out << str;
    system("PAUSE");
    exit(-1);
}

llvm::GlobalVariable* CodeGen::CreateGlob(llvm::IRBuilder<>& builder, const std::string name, llvm::Type* ty) {
    the_module->getOrInsertGlobal(name, ty);
    auto g_var = the_module->getNamedGlobal(name);
    g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
    g_var->setAlignment(4);
    return g_var;
}

llvm::Function* CodeGen::CreateFunc(llvm::IRBuilder<>& builder, const std::string name) {
    const auto func_type = llvm::FunctionType::get(builder.getInt32Ty(), false);
    const auto foo_func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, name,
                                                 the_module.get());
    return foo_func;
}

llvm::BasicBlock* CodeGen::CreateBb(llvm::Function* func, const std::string name) {
    return llvm::BasicBlock::Create(CodeGen::the_context, name, func);
}

llvm::AllocaInst* CodeGen::CreateEntryBlockAlloca(llvm::Function* the_function, llvm::Type* type,
                                              const std::string& var_name) {
    llvm::IRBuilder<> tmp_b(&the_function->getEntryBlock(), the_function->getEntryBlock().begin());
    return tmp_b.CreateAlloca(type, 0, var_name);
}

std::string CodeGen::MangleStr(const std::wstring str) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().
        to_bytes(str);
}

llvm::Type* CodeGen::GetType(std::wstring type_name) {
    if (type_name.size() != 1)return the_module->getTypeByName(MangleStr(type_name))->getPointerTo();
    const int type = type_name[0];
    switch (type) {
    case '0': return llvm::Type::getVoidTy(CodeGen::the_context);
    case K_int: return llvm::Type::getInt32Ty(CodeGen::the_context);
    case K_float: return llvm::Type::getFloatTy(CodeGen::the_context);
    case K_double: return llvm::Type::getDoubleTy(CodeGen::the_context);
    case K_bool: return llvm::Type::getInt1Ty(CodeGen::the_context);
    case K_string: return llvm::Type::getInt8PtrTy(CodeGen::the_context);
	default: return nullptr;
    }
}

llvm::StoreInst* CodeGen::AlignStore(llvm::StoreInst* a) {
    // a->setAlignment(MaybeAlign(8));
    return a;
}

llvm::LoadInst* CodeGen::AlignLoad(llvm::LoadInst* a) {
    // a->setAlignment(MaybeAlign(8));
    return a;
}

void CodeGen::BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg) {
    llvm::Function::Create(llvm::FunctionType::get(ret, types, isVarArg), llvm::Function::ExternalLinkage, name,
                           the_module.get());
}

void CodeGen::WriteReadableIr(llvm::Module* module, const char* file, bool print) {
	std::string ir;
	llvm::raw_string_ostream ir_stream(ir);
	ir_stream << *module;
	ir_stream.flush();
	std::ofstream file_stream;
	file_stream.open(file);
	file_stream << ir;
	file_stream.close();
	if (print)std::cout << ir;
}

void CodeGen::WriteBitCodeIr(llvm::Module* module, const char* file) {
	std::error_code ec;
	llvm::raw_fd_ostream os(file, ec, llvm::sys::fs::F_None);
	WriteBitcodeToFile(*module, os);
	os.flush();
}
int CodeGen::GetValuePtrDepth(llvm::Value* value) {
    auto depth = 0;
	auto type = value->getType();
    while (type->getTypeID()==llvm::Type::PointerTyID) {
		depth++;
		type = type->getPointerElementType();
    }
	return depth;
}

std::string CodeGen::GetValueStructType(llvm::Value* value) {
	auto type = value->getType();
	while (type->getTypeID() == llvm::Type::PointerTyID) 
		type = type->getPointerElementType();
	return type->getStructName();
}

std::string CodeGen::GetValueDebugType(llvm::Value* value) {
	std::string ir;
	llvm::raw_string_ostream ir_stream(ir);
	value->print(ir_stream, true);
	return ir;
}


llvm::Value* CodeGen::GetMemberField(llvm::Value* obj, const std::wstring name) {
	auto obj_type_name = obj->getType()->getPointerElementType()->getStructName().str();
	auto decl = CodeGen::types_table[obj_type_name];
	auto idx = -1;
	for (int id = 0, n = decl->fields.size(); id < n; id++) {

		if (decl->fields[id] == name) { 
			idx = id;
			break;
		}
	}
	if (idx == -1)return CodeGen::LogErrorV("Cannot find field... \n");
	const auto v = obj;

	return  CodeGen::builder.CreateStructGEP(v, idx);
}
llvm::Value* CodeGen::GetField(const std::wstring name, const bool warn) {
	llvm::Value* v = nullptr;
	const auto mangle_name = CodeGen::MangleStr(name);

	if (!v && CodeGen::local_fields_table.find(mangle_name) != CodeGen::local_fields_table.end())
		v = CodeGen::local_fields_table[mangle_name];

	if (!v && CodeGen::local_fields_table.find("this") != CodeGen::local_fields_table.end()) {
		v = CodeGen::builder.CreateLoad(CodeGen::GetMemberField(CodeGen::local_fields_table["this"], name), "this." + mangle_name);
	}

	if (!v && CodeGen::global_fields_table.find(mangle_name) != CodeGen::global_fields_table.end())
		v = CodeGen::global_fields_table[mangle_name];

	// TODO find v in public Enums
	// TODO find v in namespaces
	// TODO find v in this

	if (!v && warn) return  CodeGen::LogErrorV((std::string("Unknown variable name: ") + mangle_name + "\n").c_str());
    return v;
}
namespace parser {

	void Statements::Gen() {
		stmt1->Gen();
		stmt2->Gen();
	}

	llvm::Value* NumberConst::Gen(int cmd) {
		switch (type) {
		case K_float: return llvm::ConstantFP::get(CodeGen::the_context, llvm::APFloat(static_cast<float>(value)));
		case K_double: return llvm::ConstantFP::get(CodeGen::the_context, llvm::APFloat(value));
		case K_int: return llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), static_cast<int>(value));
		default: return CodeGen::LogErrorV("Unknown number type");
		}
	}

	llvm::Value* Boolean::Gen(int cmd) {
		return value ? CodeGen::True : CodeGen::False;;
	}

	llvm::Value* String::Gen(int cmd) {
		return CodeGen::builder.CreateGlobalStringPtr(llvm::StringRef(CodeGen::MangleStr(value)));
	}

    // c is 0 by default, when c is 1, then it keep the pointer of the value.
	llvm::Value* Field::Gen(const int c) {
	    cmd=c;
		return GenField(nullptr);
	}

	llvm::Value* FuncCall::Gen(int cmd) {
		return GenField(nullptr);
	}

	llvm::Value* Field::GenField(llvm::Value* parent) {
	
		llvm::Value* v = nullptr;
		if (parent == nullptr)v = CodeGen::GetField(name);
		else {
			parent = CodeGen::builder.CreateLoad(parent);
		    v = CodeGen::GetMemberField(parent, name);
		}

		if (child != nullptr)
		{
			child->cmd = cmd;
			v = child->GenField(v);
		}

		if (parent==nullptr) {
			if (v->getType()->getTypeID() == llvm::Type::PointerTyID && cmd == 0)
			    return CodeGen::AlignLoad(CodeGen::builder.CreateLoad(v));
		}

		return v;

    }

	llvm::Value* FuncCall::GenField(llvm::Value* parent) {
        // TODO function call like a()()

		const auto is_member_func = parent != nullptr;

        llvm::Value* v = nullptr;

		if (left != nullptr)
			v = left->GenField(parent);

        const auto callee_name = parent == nullptr
                                      ? CodeGen::MangleStr(name)
                                      : CodeGen::GetValueStructType(parent) + "." + CodeGen::MangleStr(name);


       // callee_name should now be the full function name, eg: A.bar
	   const auto callee = CodeGen::the_module->getFunction(callee_name);
	   // check if the function exist
       if (!callee)  
		   return CodeGen::LogErrorV((std::string("Unknown function referenced :")+ callee_name).c_str());
	   // check if the function argument count matchs, notes that member func pass an extra argument.
	   // some function could have varible arguments size when isVarArg is true.
       if (callee->arg_size() != (args.size()+ (is_member_func?1:0)) && !callee->isVarArg())
           return CodeGen::LogErrorV("Incorrect # arguments passed");

       // generate all the argument values;
       std::vector<llvm::Value*> args_v;
	   if (is_member_func) {
           const auto ptr_depth = CodeGen::GetValuePtrDepth(parent);
		   auto this_arg = parent;
		   if (ptr_depth == 2) // ptr_depth = 2 means it is X**, but we want X*
			   this_arg = CodeGen::builder.CreateLoad(parent);
		   args_v.push_back(this_arg);
	   }
       // prepare the argv list
       for (unsigned i = 0, e = args.size(); i != e; ++i) {
           args_v.push_back(args[i]->Gen());
           if (!args_v.back())return CodeGen::LogErrorV("Incorrect # arguments passed with error");
       }
	   // call the function
	   v= callee->getReturnType()->getTypeID() == llvm::Type::VoidTyID
	    ? CodeGen::builder.CreateCall(callee, args_v)
	    : CodeGen::builder.CreateCall(callee, args_v, "calltmp");

       // for cases like A().a or A().B(), we need to continue the DFS.
	   if (child != nullptr)
	   {
		   child->cmd = cmd;
		   v = child->GenField(v);
	   }
	   // and return the value of the whole nested field.
	   return v;
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
            return CodeGen::LogErrorV("invalid unary operator");
        }
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
		case Le :return BASIC(FCmpULE, ICmpULE)
		case Ge :return BASIC(FCmpUGE, ICmpUGE)
		case Eq :return BASIC(FCmpUEQ, ICmpEQ)
		case Ne :return BASIC(FCmpUNE, ICmpNE)

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
                          ? CodeGen::AlignLoad(CodeGen::builder.CreateLoad(rhs,"rhs"))
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

    llvm::Value* Ternary::Gen(int cmd) {
        return nullptr;
    }

    void FunctionDecl::GenHeader() {
        auto the_function = CodeGen::the_module->getFunction(CodeGen::MangleStr(name));

        if (!the_function) {
            std::vector<llvm::Type*> types;
            if (self_type != nullptr) {
                types.push_back(self_type->getPointerTo());
                args->names.insert(args->names.begin(), L"this");
            }
            for (auto i = 0; i < args->size; i++)
                types.push_back(CodeGen::GetType(args->types[i]));

            const auto func_type = llvm::FunctionType::get(CodeGen::GetType(return_type), types, args->isVarArg);
            the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, CodeGen::MangleStr(name),
                                                  CodeGen::the_module.get());

            unsigned idx = 0;
            for (auto& arg : the_function->args())
                arg.setName(CodeGen::MangleStr(args->names[idx++]));

        }
        else *Debugger::out << "function " << name << " already defined";
    }

    void FunctionDecl::Gen() {
        if (is_extern)return;
        auto function = CodeGen::the_module->getFunction(CodeGen::MangleStr(name));
        if (!function) {
            *Debugger::out << "function head not found\n";
            return;
        }
        const auto bb = llvm::BasicBlock::Create(CodeGen::the_context, CodeGen::MangleStr(name) + "_entry", function);
        CodeGen::builder.SetInsertPoint(bb);

		
		// CodeGen::This = nullptr;
        for (auto& arg : function->args())CodeGen::local_fields_table[arg.getName()] = &arg;
		// CodeGen::local_fields_table.clear();
		// CodeGen::This = nullptr;

        // CodeGen::the_function = function;
        if (statements != nullptr)statements->Gen();
		CodeGen::local_fields_table.clear();
        verifyFunction(*function);
 
    }

    void FieldDecl::Gen() {
        const auto mangled_name = CodeGen::MangleStr(name);
        const auto val = value->Gen();

        const auto ty = type.empty()? val->getType() : CodeGen::GetType(type);
        if (!val) return;

        if (constant) {
            const auto const_v = static_cast<llvm::ConstantFP*>(val);
            const auto v = CodeGen::CreateGlob(CodeGen::builder, mangled_name, CodeGen::builder.getDoubleTy());
            v->setInitializer(const_v);
		
            CodeGen::local_fields_table[mangled_name] = v;
        }
		else {
            const auto the_function = CodeGen::builder.GetInsertBlock()->getParent();
			if (the_function->getName() == "main") {
				
                // All fields in main function are stored in heap.
				const auto alloca = CodeGen::CreateEntryBlockAlloca(the_function, ty, mangled_name);
				alloca->setAlignment(llvm::MaybeAlign(8));
				CodeGen::AlignStore(CodeGen::builder.CreateStore(val, alloca));
				CodeGen::global_fields_table[mangled_name] = alloca;
			}
			else {
                // otherwise the local field store on stack.
				const auto alloca = CodeGen::CreateEntryBlockAlloca(the_function, ty, mangled_name);
				alloca->setAlignment(llvm::MaybeAlign(8));
				CodeGen::AlignStore(CodeGen::builder.CreateStore(val, alloca));
				CodeGen::local_fields_table[mangled_name] = alloca;
			}
		}
    }

    void ClassDecl::GenHeader() {
        auto the_struct = CodeGen::the_module->getTypeByName(CodeGen::MangleStr(name));
        if (!the_struct) the_struct = llvm::StructType::create(CodeGen::the_context, CodeGen::MangleStr(name));
        else *Debugger::out << "Type " << name << " already defined" << std::endl;
    }

    void ClassDecl::Gen() {
        auto the_struct = CodeGen::the_module->getTypeByName(CodeGen::MangleStr(name));
        std::vector<llvm::Type*> field_tys;
        for (const auto& type : types)field_tys.push_back(CodeGen::GetType(type));
        the_struct->setBody(field_tys);
        CodeGen::types_table[the_struct->getName().str()] = this;

        //Create a Constructor function
        const auto func_type = llvm::FunctionType::get(the_struct->getPointerTo(), std::vector<llvm::Type*>(), false);
        auto function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, CodeGen::MangleStr(name),
                                               CodeGen::the_module.get());
        const auto bb = llvm::BasicBlock::Create(CodeGen::the_context, CodeGen::MangleStr(name) + "_entry", function);
        function->setCallingConv(llvm::CallingConv::C);
        CodeGen::builder.SetInsertPoint(bb);
        //Constructor Body

        // const auto alloca = CreateEntryBlockAlloca(the_function, the_struct, "struct");
        // alloca->setAlignment(MaybeAlign(8));

        const auto ptr = CodeGen::builder.CreateCall(CodeGen::the_module->getFunction("malloc"),
                                                 llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), 32));
        auto p = CodeGen::builder.CreateCast(llvm::Instruction::BitCast, ptr, the_struct->getPointerTo());

        CodeGen::builder.CreateRet(p);
        // auto field1= CodeGen::builder.CreateStructGEP(the_struct, alloca, 1);
        // CodeGen::builder.CreateStore(llvm::ConstantInt::get(Type::getInt32Ty(CodeGen::the_context), 233),field1);
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

        cond_v = CodeGen::builder.CreateICmpEQ(cond_v, CodeGen::True, "ifcond");
        auto function = CodeGen::builder.GetInsertBlock()->getParent();

        auto then_bb = llvm::BasicBlock::Create(CodeGen::the_context, "then", function);
        auto else_bb = llvm::BasicBlock::Create(CodeGen::the_context, "else");
        const auto merge_bb = llvm::BasicBlock::Create(CodeGen::the_context, "ifcont");

        CodeGen::builder.CreateCondBr(cond_v, then_bb, else_bb);

        CodeGen::builder.SetInsertPoint(then_bb);


        stmts->Gen();


        CodeGen::builder.CreateBr(merge_bb);
        // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
        then_bb = CodeGen::builder.GetInsertBlock();

        // Emit else block.
        function->getBasicBlockList().push_back(else_bb);
        CodeGen::builder.SetInsertPoint(else_bb);

        else_stmts->Gen();


        CodeGen::builder.CreateBr(merge_bb);
        // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
        else_bb = CodeGen::builder.GetInsertBlock();

        // Emit merge block.
        function->getBasicBlockList().push_back(merge_bb);
        CodeGen::builder.SetInsertPoint(merge_bb);

        // PHINode* PN = CodeGen::builder.CreatePHI(Type::getDoubleTy(CodeGen::the_context), 2, "iftmp");
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
            CodeGen::builder.CreateRetVoid();
            return;
        }
        const auto val = value->Gen();
        if (!val) CodeGen::LogErrorV("Error in return");
        else CodeGen::builder.CreateRet(val);
    }

    void Break::Gen() {
        if(!CodeGen::is_sub_block) {
			Debugger::AlertNonBreak(L"invalid break");
            return;
        }
		CodeGen::builder.CreateBr(CodeGen::block_end);
    }

    void Continue::Gen() {
		if (!CodeGen::is_sub_block) {
			Debugger::AlertNonBreak(L"invalid continue");
			return;
		}
		CodeGen::builder.CreateBr(CodeGen::block_begin);
    }

    void Import::Gen() { }

    void While::Gen() {

		auto cond_v = condition->Gen();
		if (!cond_v) {
			Debugger::AlertNonBreak(L"Error in condititon");
			return;
		}
		cond_v = CodeGen::builder.CreateICmpEQ(cond_v, CodeGen::True, "cond");
        const auto function = CodeGen::builder.GetInsertBlock()->getParent();
        const auto while_bb = llvm::BasicBlock::Create(CodeGen::the_context, "while", function);
        const auto end_bb = llvm::BasicBlock::Create(CodeGen::the_context, "end_while");
		CodeGen::builder.SetInsertPoint(while_bb);
		CodeGen::builder.CreateCondBr(cond_v, while_bb, end_bb);
		stmts->Gen();
		CodeGen::builder.CreateBr(while_bb);
    }

    void Do::Gen() {
		auto cond_v = condition->Gen();
		if (!cond_v) {
			Debugger::AlertNonBreak(L"Error in condititon");
			return;
		}
		cond_v = CodeGen::builder.CreateICmpEQ(cond_v, CodeGen::True, "cond");
		const auto function = CodeGen::builder.GetInsertBlock()->getParent();
		const auto while_bb = llvm::BasicBlock::Create(CodeGen::the_context, "do", function);
		const auto end_bb = llvm::BasicBlock::Create(CodeGen::the_context, "end_do");
		CodeGen::builder.SetInsertPoint(while_bb);
		stmts->Gen();
		CodeGen::builder.CreateCondBr(cond_v, while_bb, end_bb);
		CodeGen::builder.CreateBr(while_bb);
    }

    void For::Gen() {

		init->Gen();

    }

    void Program::Gen() {
        CodeGen::BuildInFunc("malloc", llvm::Type::getInt8PtrTy(CodeGen::the_context),
                         std::vector<llvm::Type*>{llvm::Type::getInt32Ty(CodeGen::the_context)});
        CodeGen::BuildInFunc("free", llvm::Type::getVoidTy(CodeGen::the_context),
                         std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(CodeGen::the_context)});
        CodeGen::BuildInFunc("printf", llvm::Type::getVoidTy(CodeGen::the_context),
                         std::vector<llvm::Type*>{llvm::Type::getInt8PtrTy(CodeGen::the_context)}, true);

        for (auto& declaration : declarations)declaration->GenHeader();
        for (auto& declaration : declarations)declaration->Gen();

        const auto main_func = CodeGen::CreateFunc(CodeGen::builder, "main");
        const auto entry = CodeGen::CreateBb(main_func, "entry");
        // CodeGen::the_function = main_func;
        CodeGen::builder.SetInsertPoint(entry);

        

        for (auto& statement : statements)if (statement != nullptr)statement->Gen();

        CodeGen::builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), 0));
        verifyFunction(*main_func);
    }

}
