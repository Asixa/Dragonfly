#include "cpu-context.h"
#include "cuda.h"
#include "frontend/debug.h"
#include "AST/Type.h"
#include "AST/declarations/function-decl.h"
#include "AST/declarations/field-list.h"

void CPUContext::BuiltinCudaCheck() {
	auto zero = constant.Get(0);

	const auto func_type = llvm::FunctionType::get(builder->getVoidTy(), false);
	const auto func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, "InitCuda", module.get());
	const auto entry = llvm->CreateBasicBlock(func, "entry");
	builder->SetInsertPoint(entry);
    
    // "C: CUdevice device;"
	// const auto device = llvm->CreateEntryBlockAlloca(driver.CUdevice, "device", func);
	driver.cuda_device= llvm->CreateGlobal("cuda_device", driver.CUdevice);
	// const auto device = llvm->CreateEntryBlockAlloca(driver.CUdevice, "device", func);

    // "C: char name[128];"
	llvm::Value* name=llvm->CreateEntryBlockAlloca(llvm::ArrayType::get(llvm::Type::getInt8Ty(context), 128),"GPUName", func);
    // "C: char* namePtr=name;"
    const auto name_ptr = builder->CreateStructGEP(name, 0);
    // "C: cuInit(0);"
	builder->CreateCall(driver.init, { constant.Get(0) });
	// "C: cuDeviceGet(&device, 0);"
    builder->CreateCall(driver.device_get, { driver.cuda_device,zero });

	// builder->CreateStore(builder->CreateLoad(device), deviceGlobal);

	// "C: cuDeviceGetName(name, 128, device);"
	builder->CreateCall(driver.device_get_name, { 
		name_ptr,
	    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 128),
	    builder->CreateLoad(driver.cuda_device) });
	// "C: printf("Using CUDA Device %s\n",namePtr);"
	builder->CreateCall(llvm->GetFunction("printf"), { 
	    builder->CreateGlobalStringPtr("Using CUDA Device %s\n"),name_ptr });
    //return;

	driver.cuda_module = llvm->CreateGlobal("cuda_module", driver.CUmodule);
	driver.cuda_context = llvm->CreateGlobal("cuda_context", driver.CUcontext);


	builder->CreateCall(driver.context_create, { driver.cuda_context, zero ,  builder->CreateLoad(driver.cuda_device) });
	driver.ptx = llvm::UndefValue::get(llvm::Type::getInt8PtrTy(context));

	builder->CreateCall(driver.module_load_data_ex, { driver.cuda_module,driver.ptx, zero , zero ,
		llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(context)->getPointerTo()) });

	llvm::Constant::replaceUndefsWith(driver.ptx, builder->CreateGlobalStringPtr("lalalalalalala","cuda_ptx"));

	builder->CreateRetVoid();


    // add for user call.
	auto decl = std::make_shared<AST::decl::FunctionDecl>();
	decl->return_type = AST::BasicType::Void;
	decl->args = std::make_shared<AST::decl::FieldList>();
	decl->args->type = AST::decl::FieldList::Arguments;
	ast->AddExternFunc("InitCuda", decl);

}
void CPUContext::BuiltinAssert() {
	auto zero = constant.Get(0);
	const auto func_type = llvm::FunctionType::get(builder->getVoidTy(), { constant.int32}, false);
	const auto func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, "assert", module.get());
	const auto entry = llvm->CreateBasicBlock(func, "entry");
	builder->SetInsertPoint(entry);
    
	auto cond_v = builder->CreateICmpEQ(func->getArg(0), constant.zero, "ifcond");
	auto then_bb = llvm::BasicBlock::Create(context, "then", func);
	auto else_bb = llvm::BasicBlock::Create(context, "else", func);
	builder->CreateCondBr(cond_v, then_bb,else_bb);
	builder->SetInsertPoint(then_bb);

	llvm::Value* msg = llvm->CreateEntryBlockAlloca(llvm::ArrayType::get(constant.int32, 128), "msg", func);
	const auto msg_ptr = builder->CreateStructGEP(msg, 0);

	llvm::Value* file = llvm->CreateEntryBlockAlloca(llvm::ArrayType::get(constant.int32, 128), "file", func);
	const auto file_ptr = builder->CreateStructGEP(file, 0);
	builder->CreateCall(llvm->GetFunction("_wassert"), {
		msg_ptr,
		file_ptr,
		constant.Get(0)});
	builder->CreateRetVoid();
	builder->SetInsertPoint(else_bb);
	builder->CreateRetVoid();

	// add for user call.
	auto decl = std::make_shared<AST::decl::FunctionDecl>();
	decl->return_type = AST::BasicType::Void;
	decl->args = std::make_shared<AST::decl::FieldList>();
	decl->args->content.push_back(std::make_shared<AST::decl::FieldDecl>("value", AST::BasicType::Int));
	decl->args->type = AST::decl::FieldList::Arguments;
	ast->AddExternFunc("assert", decl);

}

CPUContext::CPUContext(std::shared_ptr<AST::Program> program):DFContext(program) {
	
	auto this_ptr = std::shared_ptr<DFContext>(this);
	ast = std::make_shared<frontend::ASTSymbol>(this_ptr);
	llvm = std::make_shared<frontend::LLVMSymbol>(this_ptr);
	constant.Init(this_ptr);
	BuiltIn(this_ptr);
}

std::shared_ptr<CPUContext> CPUContext::Create(std::shared_ptr<AST::Program> program) {
	auto ptr = std::make_shared<CPUContext>(program);
	contexts.push_back(ptr);
	return ptr;
}

void CPUContext::BuiltIn(std::shared_ptr<DFContext> ptr) {
        

	// BuildInFunc(ptr, "malloc", AST::BasicType::Void_Ptr, {AST::BasicType::Int });
	// BuildInFunc(ptr, "memcpy", AST::BasicType::Void_Ptr, {AST::BasicType::Void_Ptr, AST::BasicType::Void_Ptr, AST::BasicType::Int });
	// BuildInFunc(ptr, "free", AST::BasicType::Void_Ptr, {AST::BasicType::Void_Ptr });
	BuildInFunc(ptr, "printf", AST::BasicType::Void, { AST::BasicType::Void_Ptr }, true);
	BuildInFunc("malloc", constant.void_ptr, { constant.int32 });
	BuildInFunc("memcpy", constant.void_ptr, { constant.void_ptr,constant.void_ptr, constant.int32 });
	BuildInFunc("free", constant.void_ptr, { constant.void_ptr });
	BuildInFunc("_wassert", constant.void_ptr, { constant.int32_ptr ,constant.int32_ptr ,constant.unsigned_int });

    
	
	driver.CUcontext = BuildInType("struct.CUctx_st")->getPointerTo();
	driver.CUstream = BuildInType("struct.CUstream_st")->getPointerTo();
	driver.CUfunction = BuildInType("struct.CUfunc_st")->getPointerTo();
	driver.CUmodule = BuildInType("struct.CUmod_st")->getPointerTo();
	driver.CUevent = BuildInType("struct.CUevent_st")->getPointerTo();
	driver.CUdevicePtr = constant.size_t;
	driver.CUdevice = constant.int32;
	driver.CUmem_advise = constant.int32;
	driver.CUjit_option = constant.int32;
	driver.CUdevice_attribute = constant.int32;
	driver.CUresult = constant.int32;

	driver.init =                           BuildInFunc("cuInit",               driver.CUresult, { constant.unsigned_int });
	driver.driver_get_version =             BuildInFunc("cuDriverGetVersion",   driver.CUresult, { constant.int32_ptr });
	driver.device_get_count =               BuildInFunc("cuDeviceGetCount",     driver.CUresult, { constant.int32_ptr });
	driver.device_get =                     BuildInFunc("cuDeviceGet",          driver.CUresult, { driver.CUdevice->getPointerTo(),constant.int32 });

	driver.device_get_name =                BuildInFunc("cuDeviceGetName",      driver.CUresult, { constant.char_ptr,constant.int32,driver.CUdevice });

	driver.device_get_attribute =           BuildInFunc("cuDeviceGetAttribute", driver.CUresult, { constant.int32_ptr,constant.int32,driver.CUdevice });
	driver.context_create =                 BuildInFunc("cuCtxCreate_v2",       driver.CUresult, { driver.CUcontext->getPointerTo(),constant.unsigned_int,driver.CUdevice });
	driver.context_set_current =            BuildInFunc("cuCtxSetCurrent",      driver.CUresult, { driver.CUcontext });
	driver.context_get_current =            BuildInFunc("cuCtxGetCurrent",      driver.CUresult, { driver.CUcontext->getPointerTo() });
	driver.stream_create =                  BuildInFunc("cuStreamCreate",       driver.CUresult, { driver.CUstream->getPointerTo(),constant.unsigned_int});
	driver.memcpy_host_to_device =          BuildInFunc("cuMemcpyHtoD_v2",      driver.CUresult, { driver.CUdevicePtr,constant.void_ptr,constant.size_t });
	driver.memcpy_device_to_host =          BuildInFunc("cuMemcpyDtoH_v2",      driver.CUresult, { constant.void_ptr,driver.CUdevicePtr,constant.size_t });
	driver.memcpy_host_to_device_async =    BuildInFunc("cuMemcpyHtoDAsync_v2", driver.CUresult, { driver.CUdevicePtr, constant.void_ptr ,constant.size_t,driver.CUstream });
	driver.memcpy_device_to_host_async =    BuildInFunc("cuMemcpyDtoHAsync_v2", driver.CUresult, { constant.void_ptr ,driver.CUdevicePtr, constant.size_t,driver.CUstream });
	driver.malloc =                         BuildInFunc("cuMemAlloc_v2",        driver.CUresult, { driver.CUdevicePtr->getPointerTo() ,constant.size_t });
	driver.malloc_managed =                 BuildInFunc("cuMemAllocManaged",    driver.CUresult, { driver.CUdevicePtr->getPointerTo() ,constant.size_t,constant.unsigned_int});
	driver.memset =                         BuildInFunc("cuMemsetD8_v2",        driver.CUresult, { driver.CUdevicePtr, llvm::Type::getInt8Ty(context),constant.size_t });
	driver.mem_free =                       BuildInFunc("cuMemFree_v2",         driver.CUresult, { driver.CUdevicePtr });
	driver.mem_advise =                     BuildInFunc("cuMemAdvise",          driver.CUresult, { driver.CUdevicePtr,constant.size_t,driver.CUmem_advise ,driver.CUdevice });
	driver.mem_get_info =                   BuildInFunc("cuMemGetInfo_v2",      driver.CUresult, { constant.size_t->getPointerTo(),constant.size_t->getPointerTo() });
	driver.module_get_function =            BuildInFunc("cuModuleGetFunction",  driver.CUresult, { driver.CUfunction->getPointerTo(),driver.CUmodule,constant.char_ptr });
	driver.module_load_data_ex =            BuildInFunc("cuModuleLoadDataEx",   driver.CUresult, { driver.CUmodule->getPointerTo(),constant.void_ptr,constant.int32,driver.CUjit_option,constant.void_ptr_ptr });
	driver.launch_kernel =                  BuildInFunc("cuLaunchKernel",       driver.CUresult, { driver.CUfunction,constant.unsigned_int,constant.unsigned_int,constant.unsigned_int,constant.unsigned_int,constant.unsigned_int,constant.unsigned_int,constant.unsigned_int,driver.CUstream,constant.void_ptr_ptr,constant.void_ptr_ptr });
	driver.stream_synchronize =             BuildInFunc("cuStreamSynchronize",  driver.CUresult, { driver.CUstream });
	driver.event_create =                   BuildInFunc("cuEventCreate",        driver.CUresult, { driver.CUevent,constant.unsigned_int });
	driver.event_record =                   BuildInFunc("cuEventRecord",        driver.CUresult, { driver.CUevent,driver.CUstream });
	driver.event_elapsed_time =             BuildInFunc("cuEventElapsedTime",   driver.CUresult, { constant.float_ptr,driver.CUevent,driver.CUevent });
	BuiltinCudaCheck();
	BuiltinAssert();
}


