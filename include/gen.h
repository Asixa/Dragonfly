#ifndef CODEGEN
#define CODEGEN
#include "llvm/ADT/APFloat.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"

#include "llvm/IR/Type.h" 
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "parser.h"

using namespace lexer;
using namespace llvm;
using namespace parser;


	static LLVMContext the_context;
	static std::unique_ptr<Module> the_module = std::make_unique<Module>("Program", the_context);;
	static IRBuilder<> builder(the_context);
	static Function* main_func;
	static BasicBlock* main_block;
	static std::map<std::string, Value*> named_values;
	static std::map<std::string, StructDecl*> named_types;
	
	Value* LogErrorV(const char* str) {
		printf("%s", str);
		system("PAUSE");
		exit(-1);
	}

	inline GlobalVariable* CreateGlob(IRBuilder<>& builder, const std::string name,Type* ty) {
		the_module->getOrInsertGlobal(name,ty);
		auto g_var = the_module->getNamedGlobal(name);
		g_var->setLinkage(GlobalValue::CommonLinkage);
		g_var->setAlignment(4);
		return g_var;
	}

	inline Function* CreateFunc(IRBuilder<>& builder, const std::string name) {
		const auto func_type = FunctionType::get(builder.getInt32Ty(),false);
		const auto foo_func = Function::Create(func_type, GlobalValue::ExternalLinkage, name,the_module.get());
		return foo_func;
	}

	inline BasicBlock* CreateBb(Function* func, const std::string name) {
		return BasicBlock::Create(the_context, name, func);
	}

	static AllocaInst* CreateEntryBlockAlloca(Function* the_function, Type* type,const std::string& var_name) {
		IRBuilder<> tmp_b(&the_function->getEntryBlock(),the_function->getEntryBlock().begin());
		return tmp_b.CreateAlloca(type, 0, var_name);
	}

	inline std::string WstrToStr(const std::wstring str)
	{
		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;
		return converter.to_bytes(str);
	}

	static Type* GetType(std::wstring type_name)
	{
		auto type = 0;
		if (type_name.size() == 1)type = type_name[0];
		switch (type)
		{
		case '0':		return Type::getVoidTy(the_context);
		case Id:		return nullptr;
		case K_int:		return Type::getInt32Ty(the_context);
		case K_float:	return Type::getFloatTy(the_context);
		case K_double:	return Type::getDoubleTy(the_context);
		case K_bool:	return Type::getInt1Ty(the_context);
		case K_string:	return Type::getInt8PtrTy(the_context);
		default: return the_module->getTypeByName(WstrToStr(type_name))->getPointerTo();
		}
	}

	static StoreInst* AlignStore(StoreInst* a)
	{
		// a->setAlignment(MaybeAlign(8));
		return a;
	}
	static LoadInst* AlignLoad(LoadInst* a)
	{
		// a->setAlignment(MaybeAlign(8));
		return a;
	}

	static void BuildInFunc(const char* name,Type* ret, std::vector<Type*> types,bool isVarArg=false)
	{
		Function::Create(FunctionType::get(ret, types, isVarArg), Function::ExternalLinkage, name, the_module.get());
	}

namespace parser
{

	inline void Statements::Gen()
	{
		stmt1->Gen();
		stmt2->Gen();
	}

	inline Value* NumberConst::Gen(int cmd)
	{
		switch (type)
		{
		case K_float:return ConstantFP::get(the_context, APFloat(static_cast<float>(value)));
		case K_double:return ConstantFP::get(the_context, APFloat(value));
		case K_int:return ConstantInt::get(Type::getInt32Ty(the_context), static_cast<int>(value));
		default: return LogErrorV("Unknown number type");
		}
	}

	inline Value* Boolean::Gen(int cmd)
	{
		const auto bool_type=IntegerType::get(the_context, 1);
		return ConstantInt::get(bool_type, value);
	}
		
	inline Value* String::Gen(int cmd)
	{
		return builder.CreateGlobalStringPtr(StringRef(WstrToStr(value)));
		// return ConstantDataArray::getString(the_context, WstrToStr(value), true);
	}
		
	inline Value* Field::Gen(int cmd)
	{
		auto v = named_values[WstrToStr(names[0])];
		if (!v) LogErrorV("Unknown variable name\n");
		std::string debugname;
		for (auto i = 0; i < names.size(); i++)debugname += "_"+WstrToStr(names[i]);
		if(names.size()>1)
		{
			for (auto i=1;i<names.size();i++)
			{
				auto name = names[i];
				auto decl = named_types[v->getType()->getPointerElementType()->getPointerElementType()->getStructName().str()];
				auto idx = -1;
				for (int id=0,n= decl->fields.size(); id <n; id++)
				{
					// std::wcout << "cmp " << decl->fields[id] <<" - "<<n<<" - "<<id<< std::endl;
					if (decl->fields[id] == name)
					{
						idx = id;
						break;
					}
				}
				if (idx == -1)return LogErrorV("Cannot find field... \n");
				v = AlignLoad(builder.CreateLoad(v, debugname));
				v = builder.CreateStructGEP(v, idx);
			}
		}
		
		if (v->getType()->getTypeID()== Type::PointerTyID&& cmd == 0)
			return AlignLoad(builder.CreateLoad(v, debugname));
		return v;
	}
		
	inline Value* FuncCall::Gen(int cmd)
	{
		const auto callee = the_module->getFunction(WstrToStr(names[0]));
		if (!callee) return LogErrorV("Unknown function referenced");
		if (callee->arg_size() != args.size()&&!callee->isVarArg())
			return LogErrorV("Incorrect # arguments passed");
		std::vector<Value*> args_v;

;
		for (unsigned i = 0, e = args.size(); i != e; ++i) {
			args_v.push_back(args[i]->Gen());
			if (!args_v.back())return LogErrorV("Incorrect # arguments passed with error");
		}
		if (callee->getReturnType()->getTypeID() == Type::VoidTyID)
			return builder.CreateCall(callee, args_v);
		return builder.CreateCall(callee, args_v, "calltmp");
	}

	inline Value* Factor::Gen(int cmd)
	{
		return nullptr;
	}

	inline Value* Unary::Gen(int cmd)
	{
		const auto v= expr->Gen();
		if(!v)return nullptr;
		switch (op) {
		case '-':
		case '~':
		case '!':
		case Inc:
		case Dec:
		default:
			return LogErrorV("invalid binary operator");

		}
	}

	inline Value* Binary::Gen(int cmd)
	{
		const auto load_ptr = op == '=' || op >= AddAgn;
		auto lhs = LHS->Gen(load_ptr);
		auto rhs = RHS->Gen(load_ptr);
		if (!lhs || !rhs)return LogErrorV("  operands is NULL \n");;

		

		auto type = lhs->getType()->getTypeID();
		const auto ltype = lhs->getType()->getTypeID();
		const auto rtype = rhs->getType()->getTypeID();

		if (ltype != rtype)
		{
			if (ltype == Type::IntegerTyID) {
				if (rtype == Type::FloatTyID)
				{
					type = Type::FloatTyID;
					lhs = builder.CreateUIToFP(lhs, Type::getFloatTy(the_context));
				}
				else if (rtype == Type::DoubleTyID)
				{
					type = Type::DoubleTyID;
					lhs = builder.CreateUIToFP(lhs, Type::getDoubleTy(the_context));
				}
			}
			if (rtype == Type::IntegerTyID) {
				if (ltype == Type::FloatTyID)
				{
					type = Type::FloatTyID;
					rhs = builder.CreateUIToFP(lhs, Type::getFloatTy(the_context));
				}
				else if (ltype == Type::DoubleTyID)
				{
					type = Type::DoubleTyID;
					rhs = builder.CreateUIToFP(lhs, Type::getDoubleTy(the_context));
				}
			}
		}
		// else printf("Same %dtype %s\n",type,Token::Name(op));
		// printf("OP %d %s %d \n", lhs->getType()->getTypeID(), Token::Name(op), rhs->getType()->getTypeID());

		switch (op) {

#define BASIC(a,b,c)case a:{\
			if (type == Type::FloatTyID || type == Type::DoubleTyID)\
				return builder.Create##b(lhs, rhs, #b"_tmp");\
			if (type == Type::IntegerTyID)\
				return builder.Create##c(lhs, rhs, #c"_tmp");\
			return LogErrorV(" "#a" operation cannot apply on Non-number operands\n"); }
		BASIC('+', FAdd, Add)
		BASIC('-', FSub, Sub)
		BASIC('*', FMul, Mul)
		BASIC('/', FDiv, FDiv)
		BASIC('%', FRem, SRem)

		case And:
		case '&': {
			if (type == Type::IntegerTyID)return builder.CreateAnd(lhs, rhs, "and_tmp");
			return LogErrorV(" '&' operation cannot apply on Integer operands\n");
		}
		case '^': {
			if (type == Type::IntegerTyID)return builder.CreateXor(lhs, rhs, "xor_tmp");
			return LogErrorV(" '^' operation cannot apply on Integer operands\n");
		}
		case Or:
		case '|': {
			if (type == Type::IntegerTyID)return builder.CreateOr(lhs, rhs, "or_tmp");
			return LogErrorV(" '|' operation cannot apply on Integer operands\n");
		}

		case Shr:
		{
			if (type == Type::IntegerTyID)return builder.CreateAShr(lhs, rhs, "shr_tmp");
			return LogErrorV(" '<<' operation cannot apply on Integer operands\n");
		}
		case Shl:
		{
			if (type == Type::IntegerTyID)return builder.CreateShl(lhs, rhs, "shl_tmp");
			return LogErrorV(" '>>' operation cannot apply on Integer operands\n");
		}

#define CMP(a,b,c)case a:\
		{\
			if (type == Type::FloatTyID || type == Type::DoubleTyID)\
				return builder.CreateFCmp##b(lhs, rhs, #b"_tmp");\
			if (type == Type::IntegerTyID)\
				return builder.CreateICmp##c(lhs, rhs, #b"_tmp");\
			return LogErrorV(" "#a" operation cannot apply on Non-number operands\n");\
		}
		CMP('<', ULT, ULT)
		CMP('>', UGT, UGT)
		CMP(Le, ULE, ULE)
		CMP(Ge, UGE, UGE)
		CMP(Eq, UEQ, EQ)
		CMP(Ne, UNE, NE)

		case '=': {
			if(lhs->getType()->getTypeID()!=Type::PointerTyID)return LogErrorV("cannot reassign a constant\n");
			auto rhv = rhs;
			if(rhs->getType()->getTypeID() != lhs->getType()->getPointerElementType()->getTypeID())
				rhv = rhs->getType()->getTypeID() == Type::PointerTyID ? AlignLoad(builder.CreateLoad(rhs)) : rhs;
			AlignStore(builder.CreateStore(rhv, lhs));
			return lhs;
		}
#define BASIC_ASSGIN(a,b,c,d)case a:																					\
			{																											\
				auto rhv = rhs->getType()->getTypeID() == Type::PointerTyID ? AlignLoad(builder.CreateLoad(rhs)) : rhs;			\
				if (type == Type::PointerTyID)																			\
				{																										\
					type = lhs->getType()->getPointerElementType()->getTypeID();										\
					const auto lhsv = AlignLoad(builder.CreateLoad(lhs));															\
					if (type == Type::FloatTyID || type == Type::DoubleTyID)											\
						return AlignStore(builder.CreateStore(builder.Create##b(lhsv, rhv, #b"_tmp"), lhs));						\
					if (type == Type::IntegerTyID)																		\
						return AlignStore(builder.CreateStore(builder.Create##c(lhsv, rhv, #c"_tmp"), lhs));						\
					return LogErrorV(" "#d" operation cannot apply on Non-number variables\n");							\
				}																										\
				return LogErrorV(" cannot reassign a constant\n");														\
			}
			
// #define  BITWISE_ASSGIN(a,b,c,d)case a: {\
// 			if (type == Type::IntegerTyID)\
// 				lhs = builder.Create##c(lhs, rhs, #c"_tmp"); \
// 			else return LogErrorV(" "#d" operation cannot apply on Non-number variables\n");\
// 			return lhs;}

		BASIC_ASSGIN(AddAgn,FAdd,Add,+=)
		BASIC_ASSGIN(SubAgn,FSub,Sub,-=)
		BASIC_ASSGIN(MulAgn,FMul,Mul,*=)
		BASIC_ASSGIN(DivAgn,FDiv,FDiv,/=)
		BASIC_ASSGIN(ModAgn,FRem,SRem,%=)

		
		default:
			return LogErrorV("invalid binary operator");

		}
	}

	inline Value* Ternary::Gen(int cmd)
	{
		return nullptr;
	}

	inline void FunctionDecl::GenHeader()
	{
		auto the_function = the_module->getFunction(WstrToStr(name));

		if (!the_function)
		{
			std::vector<Type*> types;
			for (auto i = 0; i < args->size; i++)
			{
				types.push_back(GetType(args->types[i]));
			};
			const auto func_type = FunctionType::get(GetType(return_type), types, args->isVarArg);
			the_function = Function::Create(func_type, Function::ExternalLinkage, WstrToStr(name), the_module.get());
			unsigned idx = 0;
			for (auto& arg : the_function->args())
			{
				arg.setName(WstrToStr(args->names[idx++]));
			}
		}
		else
		{
			printf("function %s already defined",name);
		}

	}

	inline void FunctionDecl::Gen()
	{
		if(is_extern)return;
		auto the_function = the_module->getFunction(WstrToStr(name));
		if(!the_function)
		{
			printf("function head not found\n"); return;
		}
		const auto bb = BasicBlock::Create(the_context, WstrToStr(name)+"_entry", the_function);
		builder.SetInsertPoint(bb);
		
		named_values.clear();
		for (auto& arg : the_function->args())named_values[arg.getName()] = &arg;
	
		if (statements != nullptr)statements->Gen();

		verifyFunction(*the_function);
		return;
		// if (Value * RetVal = body->Gen()) {
		// 	// Finish off the function.
		// 	builder.CreateRet(RetVal);
		// 	// Validate the generated code, checking for consistency.
		// 	verifyFunction(*the_function);
		// 	return;
		// }

		// Error reading body, remove function.
		the_function->eraseFromParent();
		return;
	}

	inline void FieldDecl::Gen()
	{
		const auto _name = WstrToStr(name);
		const auto val = value->Gen();
		
		 auto ty = type.size() == 0 ? val->getType() : GetType(type);
		if (!val) return;

		if (constant) {
			const auto const_v = static_cast<ConstantFP*>(val);
			const auto v = CreateGlob(builder, _name, builder.getDoubleTy());
			v->setInitializer(const_v);
			named_values[_name] = v;
		}
		else {
			const auto alloca = CreateEntryBlockAlloca(main_func, ty, _name);
			alloca->setAlignment(MaybeAlign(8));

			AlignStore(builder.CreateStore(val, alloca));
			named_values[_name] = alloca;
		}
		
	}

	inline void StructDecl::GenHeader()
	{
		auto the_struct= the_module->getTypeByName(WstrToStr(name));
		if (!the_struct) the_struct = StructType::create(the_context,  WstrToStr(name));
		else std::wcout << "Type " << name << " already defined" << std::endl;
	}

	inline void StructDecl::Gen()
	{
		auto the_struct = the_module->getTypeByName(WstrToStr(name));
		std::vector<Type*> field_tys;
		for (const auto& type : types)field_tys.push_back(GetType(type));
		the_struct->setBody(field_tys);
		named_types[the_struct->getName().str()] = this;

		//Create a Constructor function
		const auto func_type = FunctionType::get(the_struct->getPointerTo(), std::vector<Type*>(), false);
		auto the_function = Function::Create(func_type, Function::ExternalLinkage, WstrToStr(name), the_module.get());
		const auto bb = BasicBlock::Create(the_context, WstrToStr(name) + "_entry", the_function);
		the_function->setCallingConv(llvm::CallingConv::C);
		builder.SetInsertPoint(bb);
		//Constructor Body
		
		// const auto alloca = CreateEntryBlockAlloca(the_function, the_struct, "struct");
		// alloca->setAlignment(MaybeAlign(8));

		auto ptr=builder.CreateCall(the_module->getFunction("malloc"), 
			ConstantInt::get(Type::getInt32Ty(the_context), 32));
		auto p=builder.CreateCast(Instruction::BitCast, ptr, the_struct->getPointerTo());

		builder.CreateRet(p);
		// auto field1= builder.CreateStructGEP(the_struct, alloca, 1);
		// builder.CreateStore(ConstantInt::get(Type::getInt32Ty(the_context), 233),field1);

		
		verifyFunction(*the_function);
	}

	inline void If::Gen()
	{
		// auto cond_v = condition->Gen();
		// if (!cond_v)return nullptr;
		//
		// cond_v = Builder.CreateFCmpONE(cond_v, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");
		//
		// auto TheFunction = Builder.GetInsertBlock()->getParent();
		//
		// auto ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
		// auto ElseBB = BasicBlock::Create(TheContext, "else");
		// auto MergeBB = BasicBlock::Create(TheContext, "ifcont");
		//
		// Builder.CreateCondBr(cond_v, ThenBB, ElseBB);
		//
		// Builder.SetInsertPoint(ThenBB);
		//
		//
		// Value* ThenV = Then->codegen();
		// if (!ThenV)
		// 	return nullptr;
		//
		// Builder.CreateBr(MergeBB);
		// // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
		// ThenBB = Builder.GetInsertBlock();
		//
		// // Emit else block.
		// TheFunction->getBasicBlockList().push_back(ElseBB);
		// Builder.SetInsertPoint(ElseBB);
		//
		// Value* ElseV = Else->codegen();
		// if (!ElseV)
		// 	return nullptr;
		//
		// Builder.CreateBr(MergeBB);
		// // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
		// ElseBB = Builder.GetInsertBlock();
		//
		// // Emit merge block.
		// TheFunction->getBasicBlockList().push_back(MergeBB);
		// Builder.SetInsertPoint(MergeBB);
		// PHINode* PN = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");
		//
		// PN->addIncoming(ThenV, ThenBB);
		// PN->addIncoming(ElseV, ElseBB);
		// return PN;
		//
		return;
		
	}

	inline void Empty::Gen()
	{
		value->Gen();
	}

	inline void Throw::Gen()
	{
	}

	inline void Return::Gen()
	{
		const auto val = value->Gen();
		if (!val) LogErrorV("Error in return");
		else builder.CreateRet(val);
	}

	inline void Break::Gen()
	{
	}

	inline void Continue::Gen()
	{
	}

	inline void Import::Gen()
	{
	}

	inline void Program::Gen()
	{
		BuildInFunc("malloc", Type::getInt8PtrTy(the_context), std::vector<Type*>{ Type::getInt32Ty(the_context) });
		BuildInFunc("free", Type::getVoidTy(the_context), std::vector<Type*>{ Type::getInt8PtrTy(the_context) });
		BuildInFunc("printf", Type::getVoidTy(the_context), std::vector<Type*>{ Type::getInt8PtrTy(the_context)->getPointerTo() });

		for (auto& declaration : declarations)declaration->GenHeader();
		for (auto& declaration : declarations)declaration->Gen();
		
		main_func = CreateFunc(builder, "main");
		const auto entry = CreateBb(main_func, "entry");
		builder.SetInsertPoint(entry);
		
		for (auto& statement : statements)if(statement!=nullptr)statement->Gen();
		
		builder.CreateRet(ConstantInt::get(Type::getInt32Ty(the_context), 0));
		verifyFunction(*main_func);
	}
}
#endif
