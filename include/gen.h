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






	static llvm::LLVMContext the_context;
	static std::unique_ptr<llvm::Module> the_module = std::make_unique<llvm::Module>("Program", the_context);
	static llvm::IRBuilder<> builder(the_context);
	static llvm::Function* the_function;
	static std::map<std::string, llvm::Value*> named_values;
	static std::map<std::string, parser::ClassDecl*> named_types;

	static llvm::Value* True = llvm::ConstantInt::get(llvm::IntegerType::get(the_context, 1),1);
	static llvm::Value* False = llvm::ConstantInt::get(llvm::IntegerType::get(the_context, 1),0);

	llvm::Value* LogErrorV(const char* str) {
		*debugger::out<<str;
		system("PAUSE");
		exit(-1);
	}

	inline llvm::GlobalVariable* CreateGlob(llvm::IRBuilder<>& builder, const std::string name, llvm::Type* ty) {
		the_module->getOrInsertGlobal(name,ty);
		auto g_var = the_module->getNamedGlobal(name);
		g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
		g_var->setAlignment(4);
		return g_var;
	}

	inline llvm::Function* CreateFunc(llvm::IRBuilder<>& builder, const std::string name) {
		const auto func_type = llvm::FunctionType::get(builder.getInt32Ty(),false);
		const auto foo_func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, name,the_module.get());
		return foo_func;
	}

	inline llvm::BasicBlock* CreateBb(llvm::Function* func, const std::string name) {
		return llvm::BasicBlock::Create(the_context, name, func);
	}

	static llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* the_function, llvm::Type* type,const std::string& var_name) {
        llvm::IRBuilder<> tmp_b(&the_function->getEntryBlock(),the_function->getEntryBlock().begin());
		return tmp_b.CreateAlloca(type, 0, var_name);
	}

	inline std::string WstrToStr(const std::wstring str)
	{
		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;
		return converter.to_bytes(str);
	}

	static llvm::Type* GetType(std::wstring type_name)
	{
		auto type = 0;
		if (type_name.size() == 1)type = type_name[0];
		switch (type)
		{
		case '0':		return llvm::Type::getVoidTy(the_context);
		case Id:		return nullptr;
		case K_int:		return llvm::Type::getInt32Ty(the_context);
		case K_float:	return llvm::Type::getFloatTy(the_context);
		case K_double:	return llvm::Type::getDoubleTy(the_context);
		case K_bool:	return llvm::Type::getInt1Ty(the_context);
		case K_string:	return llvm::Type::getInt8PtrTy(the_context);
		default: return the_module->getTypeByName(WstrToStr(type_name))->getPointerTo();
		}
	}

	static llvm::StoreInst* AlignStore(llvm::StoreInst* a)
	{
		// a->setAlignment(MaybeAlign(8));
  	return a;
	}
	static llvm::LoadInst* AlignLoad(llvm::LoadInst* a)
	{
		// a->setAlignment(MaybeAlign(8));
		return a;
	}

	static void BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types,bool isVarArg=false)
	{
		llvm::Function::Create(llvm::FunctionType::get(ret, types, isVarArg), llvm::Function::ExternalLinkage, name, the_module.get());
	}

namespace parser
{

	inline void Statements::Gen()
	{
		stmt1->Gen();
		stmt2->Gen();
	}

	inline llvm::Value* NumberConst::Gen(int cmd)
	{
		switch (type)
		{
		case K_float:return llvm::ConstantFP::get(the_context, llvm::APFloat(static_cast<float>(value)));
		case K_double:return llvm::ConstantFP::get(the_context, llvm::APFloat(value));
		case K_int:return llvm::ConstantInt::get(llvm::Type::getInt32Ty(the_context), static_cast<int>(value));
		default: return LogErrorV("Unknown number type");
		}
	}

	inline llvm::Value* Boolean::Gen(int cmd)
	{
		return value ? True : False;
		// const auto bool_type=llvm::IntegerType::get(the_context, 1);
		// return llvm::ConstantInt::get(bool_type, value);
	}
		
	inline llvm::Value* String::Gen(int cmd)
	{
		return builder.CreateGlobalStringPtr(llvm::StringRef(WstrToStr(value)));
		// return ConstantDataArray::getString(the_context, WstrToStr(value), true);
	}
		
	inline llvm::Value* Field::Gen(int cmd)
	{
		auto v = named_values[WstrToStr(names[0])];
		if (!v) LogErrorV("Unknown variable name\n");
		std::string debugname;
		for (const auto& name : names)debugname += "_"+WstrToStr(name);
		if(names.size()>1)
		{
			for (auto i=1;i<names.size();i++)
			{
				auto name = names[i];
				auto decl = named_types[v->getType()->getPointerElementType()->getPointerElementType()->getStructName().str()];
				auto idx = -1;
				for (int id=0,n= decl->fields.size(); id <n; id++)
				{

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
		
		if (v->getType()->getTypeID()== llvm::Type::PointerTyID&& cmd == 0)
			return AlignLoad(builder.CreateLoad(v, debugname));
		return v;
	}
		
	inline llvm::Value* FuncCall::Gen(int cmd)
	{
		const auto callee = the_module->getFunction(WstrToStr(names[0]));
		if (!callee) return LogErrorV("Unknown function referenced");
		if (callee->arg_size() != args.size()&&!callee->isVarArg())
			return LogErrorV("Incorrect # arguments passed");
		std::vector<llvm::Value*> args_v;

;
		for (unsigned i = 0, e = args.size(); i != e; ++i) {
			args_v.push_back(args[i]->Gen());
			if (!args_v.back())return LogErrorV("Incorrect # arguments passed with error");
		}
		if (callee->getReturnType()->getTypeID() == llvm::Type::VoidTyID)
			return builder.CreateCall(callee, args_v);
		return builder.CreateCall(callee, args_v, "calltmp");
	}

	inline llvm::Value* Factor::Gen(int cmd)
	{
		return nullptr;
	}

	inline llvm::Value* Unary::Gen(int cmd)
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

	inline llvm::Value* Binary::Gen(int cmd)
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
			if (ltype == llvm::Type::IntegerTyID) {
				if (rtype == llvm::Type::FloatTyID)
				{
					type = llvm::Type::FloatTyID;
					lhs = builder.CreateUIToFP(lhs, llvm::Type::getFloatTy(the_context));
				}
				else if (rtype == llvm::Type::DoubleTyID)
				{
					type = llvm::Type::DoubleTyID;
					lhs = builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(the_context));
				}
			}
			if (rtype == llvm::Type::IntegerTyID) {
				if (ltype == llvm::Type::FloatTyID)
				{
					type = llvm::Type::FloatTyID;
					rhs = builder.CreateUIToFP(lhs, llvm::Type::getFloatTy(the_context));
				}
				else if (ltype == llvm::Type::DoubleTyID)
				{
					type = llvm::Type::DoubleTyID;
					rhs = builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(the_context));
				}
			}
		}


		switch (op) {

#define BASIC(a,b,c)case a:{\
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)\
				return builder.Create##b(lhs, rhs, #b"_tmp");\
			if (type == llvm::Type::IntegerTyID)\
				return builder.Create##c(lhs, rhs, #c"_tmp");\
			return LogErrorV(" "#a" operation cannot apply on Non-number operands\n"); }
			
		BASIC('+', FAdd, Add)
		BASIC('-', FSub, Sub)
		BASIC('*', FMul, Mul)
		
		BASIC('%', FRem, SRem)
		case '/':
		{
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID) return builder.CreateFDiv(lhs, rhs, "FDiv""_tmp");
			if (type == llvm::Type::IntegerTyID)
				return builder.CreateFDiv(
					builder.CreateCast(llvm::Instruction::SIToFP,lhs, llvm::Type::getDoubleTy(the_context)),
					builder.CreateCast(llvm::Instruction::SIToFP, rhs, llvm::Type::getDoubleTy(the_context)),
					"FDiv""_tmp");
			return LogErrorV(" ""'/'"" operation cannot apply on Non-number operands\n");
		}
		case And:
		case '&': {
			if (type == llvm::Type::IntegerTyID)return builder.CreateAnd(lhs, rhs, "and_tmp");
			return LogErrorV(" '&' operation cannot apply on Integer operands\n");
		}
		case '^': {
			if (type == llvm::Type::IntegerTyID)return builder.CreateXor(lhs, rhs, "xor_tmp");
			return LogErrorV(" '^' operation cannot apply on Integer operands\n");
		}
		case Or:
		case '|': {
			if (type == llvm::Type::IntegerTyID)return builder.CreateOr(lhs, rhs, "or_tmp");
			return LogErrorV(" '|' operation cannot apply on Integer operands\n");
		}
		case BAndAgn:
			{
				
			}
		case BXORAgn:
		{

		}case BORAgn:
		{

		}
		case Shr:
		{
			if (type == llvm::Type::IntegerTyID)return builder.CreateAShr(lhs, rhs, "shr_tmp");
			return LogErrorV(" '<<' operation cannot apply on Integer operands\n");
		}
		case Shl:
		{
			if (type == llvm::Type::IntegerTyID)return builder.CreateShl(lhs, rhs, "shl_tmp");
			return LogErrorV(" '>>' operation cannot apply on Integer operands\n");
		}

#define CMP(a,b,c)case a:\
		{\
			if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)\
				return builder.CreateFCmp##b(lhs, rhs, #b"_tmp");\
			if (type == llvm::Type::IntegerTyID)\
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
			if(lhs->getType()->getTypeID()!= llvm::Type::PointerTyID)return LogErrorV("cannot reassign a constant\n");
			auto rhv = rhs;
			if(rhs->getType()->getTypeID() != lhs->getType()->getPointerElementType()->getTypeID())
				rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID ? AlignLoad(builder.CreateLoad(rhs)) : rhs;
			AlignStore(builder.CreateStore(rhv, lhs));
			return lhs;
		}
#define BASIC_ASSGIN(a,b,c,d)case a:																					\
			{																											\
				auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID ? AlignLoad(builder.CreateLoad(rhs)) : rhs;			\
				if (type == llvm::Type::PointerTyID)																			\
				{																										\
					type = lhs->getType()->getPointerElementType()->getTypeID();										\
					const auto lhsv = AlignLoad(builder.CreateLoad(lhs));															\
					if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)											\
						return AlignStore(builder.CreateStore(builder.Create##b(lhsv, rhv, #b"_tmp"), lhs));						\
					if (type == llvm::Type::IntegerTyID)																		\
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
		BASIC_ASSGIN(ModAgn,FRem,SRem,%=)

		case DivAgn:
		{
			auto rhv = rhs->getType()->getTypeID() == llvm::Type::PointerTyID ? AlignLoad(builder.CreateLoad(rhs)) : rhs;
			if (type == llvm::Type::PointerTyID) {
				type = lhs->getType()->getPointerElementType()->getTypeID();
				const auto lhsv = AlignLoad(builder.CreateLoad(lhs));
				if (type == llvm::Type::FloatTyID || type == llvm::Type::DoubleTyID)
					return AlignStore(builder.CreateStore(builder.CreateFDiv(lhsv, rhv, "FDiv""_tmp"), lhs));
				if (type == llvm::Type::IntegerTyID)
				{
					const auto lhv_d = builder.CreateCast(llvm::Instruction::SIToFP, lhsv, llvm::Type::getDoubleTy(the_context));
					const auto rhv_d = builder.CreateCast(llvm::Instruction::SIToFP, rhv, llvm::Type::getDoubleTy(the_context));
					const auto div=builder.CreateFDiv(lhv_d, rhv_d, "div_tmp");
					const auto div_i = builder.CreateCast(llvm::Instruction::FPToUI, div, llvm::Type::getInt32Ty(the_context));
					return AlignStore(builder.CreateStore(div_i, lhs));
				}
					
					
					
					
					
				return LogErrorV(" ""/="" operation cannot apply on Non-number variables\n");
			} return LogErrorV(" cannot reassign a constant\n");
		}
		default:
			return LogErrorV("invalid binary operator");

		}
	}

	inline llvm::Value* Ternary::Gen(int cmd)
	{
		return nullptr;
	}

	inline void FunctionDecl::GenHeader()
	{
		auto the_function = the_module->getFunction(WstrToStr(name));

		if (!the_function)
		{
			std::vector<llvm::Type*> types;
			if (self_type != nullptr) {
				types.push_back(self_type->getPointerTo());
				args->names.insert(args->names.begin(), L"this");
			}
			for (auto i = 0; i < args->size; i++)
				types.push_back(GetType(args->types[i]));
			
			const auto func_type = llvm::FunctionType::get(GetType(return_type), types, args->isVarArg);
			the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, WstrToStr(name), the_module.get());

			unsigned idx = 0;
			for (auto& arg : the_function->args())
				arg.setName(WstrToStr(args->names[idx++]));
			
		}
		else *debugger::out<<"function "<< name<<" already defined";
	}

	inline void FunctionDecl::Gen()
	{
		if(is_extern)return;
		auto function = the_module->getFunction(WstrToStr(name));
		if(!function)
		{
			*debugger::out<<"function head not found\n"; return;
		}
		const auto bb = llvm::BasicBlock::Create(the_context, WstrToStr(name)+"_entry", function);
		builder.SetInsertPoint(bb);
		
		
		named_values.clear();
		for (auto& arg : function->args())named_values[arg.getName()] = &arg;

		the_function=function;
		if (statements != nullptr)statements->Gen();

		verifyFunction(*function);
		return;
	}

	inline void FieldDecl::Gen()
	{
		const auto _name = WstrToStr(name);
		const auto val = value->Gen();
		
		 auto ty = type.size() == 0 ? val->getType() : GetType(type);
		if (!val) return;

		if (constant) {
			const auto const_v = static_cast<llvm::ConstantFP*>(val);
			const auto v = CreateGlob(builder, _name, builder.getDoubleTy());
			v->setInitializer(const_v);
			named_values[_name] = v;
		}
		else {
			const auto alloca = CreateEntryBlockAlloca(the_function, ty, _name);
			alloca->setAlignment(llvm::MaybeAlign(8));

			AlignStore(builder.CreateStore(val, alloca));
			named_values[_name] = alloca;
		}
		
	}

	inline void ClassDecl::GenHeader()
	{
		auto the_struct= the_module->getTypeByName(WstrToStr(name));
		if (!the_struct) the_struct = llvm::StructType::create(the_context,  WstrToStr(name));
		else *debugger::out<<"Type " << name << " already defined" << std::endl;
	}

	inline void ClassDecl::Gen()
	{
		auto the_struct = the_module->getTypeByName(WstrToStr(name));
		std::vector<llvm::Type*> field_tys;
		for (const auto& type : types)field_tys.push_back(GetType(type));
		the_struct->setBody(field_tys);
		named_types[the_struct->getName().str()] = this;

		//Create a Constructor function
		const auto func_type = llvm::FunctionType::get(the_struct->getPointerTo(), std::vector<llvm::Type*>(), false);
		auto function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, WstrToStr(name), the_module.get());
		const auto bb = llvm::BasicBlock::Create(the_context, WstrToStr(name) + "_entry", function);
		function->setCallingConv(llvm::CallingConv::C);
		builder.SetInsertPoint(bb);
		//Constructor Body
		
		// const auto alloca = CreateEntryBlockAlloca(the_function, the_struct, "struct");
		// alloca->setAlignment(MaybeAlign(8));

    const auto ptr=builder.CreateCall(the_module->getFunction("malloc"), 
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(the_context), 32));
		auto p=builder.CreateCast(llvm::Instruction::BitCast, ptr, the_struct->getPointerTo());

		builder.CreateRet(p);
		// auto field1= builder.CreateStructGEP(the_struct, alloca, 1);
		// builder.CreateStore(llvm::ConstantInt::get(Type::getInt32Ty(the_context), 233),field1);
		verifyFunction(*function);

		for (auto& function : functions)
		{
			function->SetInternal(name,the_struct);
			function->GenHeader();
			function->Gen();
		}
	}

	inline void If::Gen()
	{
		auto cond_v = condition->Gen();
		if (!cond_v) { debugger::AlertNonBreak(L"Error in condititon"); return; }
		
		cond_v = builder.CreateICmpEQ(cond_v, True, "ifcond");
		auto function = builder.GetInsertBlock()->getParent();
		
		auto ThenBB = llvm::BasicBlock::Create(the_context, "then",function);
		auto ElseBB = llvm::BasicBlock::Create(the_context, "else");
		auto MergeBB = llvm::BasicBlock::Create(the_context, "ifcont");
		printf("what\n");
		builder.CreateCondBr(cond_v, ThenBB, ElseBB);
		
		builder.SetInsertPoint(ThenBB);
		
		
		stmts->Gen();

		
		builder.CreateBr(MergeBB);
		// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
		ThenBB = builder.GetInsertBlock();
		
		// Emit else block.
		function->getBasicBlockList().push_back(ElseBB);
		builder.SetInsertPoint(ElseBB);
		
		else_stmts->Gen();

		
		builder.CreateBr(MergeBB);
		// Codegen of 'Else' can change the current block, update ElseBB for the PHI.
		ElseBB = builder.GetInsertBlock();
		
		// Emit merge block.
		function->getBasicBlockList().push_back(MergeBB);
		builder.SetInsertPoint(MergeBB);
		
		// PHINode* PN = builder.CreatePHI(Type::getDoubleTy(the_context), 2, "iftmp");
		// PN->addIncoming(ThenV, ThenBB);
		// PN->addIncoming(ElseV, ElseBB);
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
		if (value == nullptr) {
			builder.CreateRetVoid();
			return;
		}
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

	inline void While::Gen()
	{
		
	}
	inline void Do::Gen()
	{
		
	}
	inline void For::Gen()
	{

	}
		
	inline void Program::Gen()
	{
		BuildInFunc("malloc", llvm::Type::getInt8PtrTy(the_context), std::vector<llvm::Type*>{ llvm::Type::getInt32Ty(the_context) });
		BuildInFunc("free", llvm::Type::getVoidTy(the_context), std::vector<llvm::Type*>{ llvm::Type::getInt8PtrTy(the_context) });
		BuildInFunc("printf", llvm::Type::getVoidTy(the_context), std::vector<llvm::Type*>{ llvm::Type::getInt8PtrTy(the_context) },true);

		for (auto& declaration : declarations)declaration->GenHeader();
		for (auto& declaration : declarations)declaration->Gen();
		
		auto main_func = CreateFunc(builder, "main");
		const auto entry = CreateBb(main_func, "entry");
		the_function = main_func;
		builder.SetInsertPoint(entry);
		
		for (auto& statement : statements)if(statement!=nullptr)statement->Gen();
		
		builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(the_context), 0));
		verifyFunction(*main_func);
	}
}
#endif
