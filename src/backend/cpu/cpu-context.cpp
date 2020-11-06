#include "cpu-context.h"
#include "cuda.h"
#include "frontend/debug.h"
#include "AST/Type.h"
#include "AST/declarations/function-decl.h"
#include "AST/declarations/field-list.h"

void CPUContext::BuiltinCudaCheck() {
	const auto func_type = llvm::FunctionType::get(builder->getVoidTy(), false);
	const auto func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, "CudaCheck", module.get());
	const auto entry = llvm->CreateBasicBlock(func, "entry");
	builder->SetInsertPoint(entry);

    // CUdevice device;
	// const auto device = llvm->CreateEntryBlockAlloca(driver.CUdevice, "device", func);
	const auto device= llvm->CreateGlobal("device", driver.CUdevice);
	// const auto device = llvm->CreateEntryBlockAlloca(driver.CUdevice, "device", func);

    // char name[128];
	llvm::Value* name=llvm->CreateEntryBlockAlloca(llvm::ArrayType::get(llvm::Type::getInt8Ty(context), 128),"GPUName", func);
    // char* namePtr=name;
    const auto name_ptr = builder->CreateStructGEP(name, 0);
    // cuInit(0);
	builder->CreateCall(driver.init, { llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0) });
	// cuDeviceGet(&device, 0)
    builder->CreateCall(driver.device_get, { device,llvm::ConstantInt::get(llvm::Type::getInt32Ty(context),0) });

	// builder->CreateStore(builder->CreateLoad(device), deviceGlobal);

	// cuDeviceGetName(name, 128, device);
	builder->CreateCall(driver.device_get_name, { 
		name_ptr,
	    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 128),
	    builder->CreateLoad(device) });
	// printf("Using CUDA Device %s\n",namePtr);
	builder->CreateCall(llvm->GetFunction("printf"), { 
	    builder->CreateGlobalStringPtr("Using CUDA Device %s\n"),name_ptr });
    //return;
	builder->CreateRetVoid();


    // add for user call.
	auto decl = std::make_shared<AST::decl::FunctionDecl>();
	decl->return_type = AST::BasicType::Void;
	decl->args = std::make_shared<AST::decl::FieldList>();
	decl->args->type = AST::decl::FieldList::Arguments;
	ast->AddExternFunc("CudaCheck", decl);

}

CPUContext::CPUContext(std::shared_ptr<AST::Program> program):DFContext(program) {
	
	auto this_ptr = std::shared_ptr<DFContext>(this);
	ast = std::make_shared<frontend::ASTSymbol>(this_ptr);
	llvm = std::make_shared<frontend::LLVMSymbol>(this_ptr);
	BuiltIn(this_ptr);
}

std::shared_ptr<CPUContext> CPUContext::Create(std::shared_ptr<AST::Program> program) {
	auto ptr = std::make_shared<CPUContext>(program);
	contexts.push_back(ptr);
	return ptr;
}

void CPUContext::BuiltIn(std::shared_ptr<DFContext> ptr) {
	BuildInFunc(ptr, "malloc", AST::BasicType::Void_Ptr, {AST::BasicType::Int });
	BuildInFunc(ptr, "memcpy", AST::BasicType::Void_Ptr, {AST::BasicType::Void_Ptr, AST::BasicType::Void_Ptr, AST::BasicType::Int });
	BuildInFunc(ptr, "free", AST::BasicType::Void_Ptr, {AST::BasicType::Void_Ptr });
	BuildInFunc(ptr, "printf", AST::BasicType::Void, { AST::BasicType::Void_Ptr }, true);

	const auto int32 = llvm::Type::getInt32Ty(context);
	const auto size_t = llvm::Type::getInt64Ty(context);
	const auto floatPtr = llvm::Type::getFloatPtrTy(context);
	const auto charPtr = llvm::Type::getInt8PtrTy(context);
	const auto int32Ptr = llvm::Type::getInt32PtrTy(context);
	const auto voidPtr = int32Ptr;
	const auto unsign = int32;
	const auto voidPtrPtr = int32Ptr->getPointerTo();
    
	
	driver.CUcontext = BuildInType("struct.CUctx_st")->getPointerTo();
	driver.CUstream = BuildInType("struct.CUstream_st")->getPointerTo();
	driver.CUfunction = BuildInType("struct.CUfunc_st")->getPointerTo();
	driver.CUmodule = BuildInType("struct.CUmod_st")->getPointerTo();
	driver.CUevent = BuildInType("struct.CUevent_st")->getPointerTo();
	driver.CUdevicePtr = size_t;
	driver.CUdevice = int32;
	driver.CUmem_advise = int32;
	driver.CUjit_option = int32;
	driver.CUdevice_attribute = int32;
	driver.CUresult = int32;

	driver.init =                           BuildInFunc("cuInit",               driver.CUresult, { unsign });
	driver.driver_get_version =             BuildInFunc("cuDriverGetVersion",   driver.CUresult, { int32Ptr });
	driver.device_get_count =               BuildInFunc("cuDeviceGetCount",     driver.CUresult, { int32Ptr });
	driver.device_get =                     BuildInFunc("cuDeviceGet",          driver.CUresult, { driver.CUdevice->getPointerTo(),int32 });

	driver.device_get_name =                BuildInFunc("cuDeviceGetName",      driver.CUresult, {charPtr,int32,driver.CUdevice });

	driver.device_get_attribute =           BuildInFunc("cuDeviceGetAttribute", driver.CUresult, { int32Ptr,int32,driver.CUdevice });
	driver.context_create =                 BuildInFunc("cuCtxCreate_v2",       driver.CUresult, { driver.CUcontext->getPointerTo(),unsign,driver.CUdevice });
	driver.context_set_current =            BuildInFunc("cuCtxSetCurrent",      driver.CUresult, { driver.CUcontext });
	driver.context_get_current =            BuildInFunc("cuCtxGetCurrent",      driver.CUresult, { driver.CUcontext->getPointerTo() });
	driver.stream_create =                  BuildInFunc("cuStreamCreate",       driver.CUresult, { driver.CUstream->getPointerTo(),unsign});
	driver.memcpy_host_to_device =          BuildInFunc("cuMemcpyHtoD_v2",      driver.CUresult, { driver.CUdevicePtr,voidPtr,size_t });
	driver.memcpy_device_to_host =          BuildInFunc("cuMemcpyDtoH_v2",      driver.CUresult, { voidPtr,driver.CUdevicePtr,size_t });
	driver.memcpy_host_to_device_async =    BuildInFunc("cuMemcpyHtoDAsync_v2", driver.CUresult, { driver.CUdevicePtr, voidPtr ,size_t,driver.CUstream });
	driver.memcpy_device_to_host_async =    BuildInFunc("cuMemcpyDtoHAsync_v2", driver.CUresult, { voidPtr ,driver.CUdevicePtr, size_t,driver.CUstream });
	driver.malloc =                         BuildInFunc("cuMemAlloc_v2",        driver.CUresult, { driver.CUdevicePtr->getPointerTo() ,size_t });
	driver.malloc_managed =                 BuildInFunc("cuMemAllocManaged",    driver.CUresult, { driver.CUdevicePtr->getPointerTo() ,size_t,unsign});
	driver.memset =                         BuildInFunc("cuMemsetD8_v2",        driver.CUresult, { driver.CUdevicePtr, llvm::Type::getInt8Ty(context),size_t });
	driver.mem_free =                       BuildInFunc("cuMemFree_v2",         driver.CUresult, { driver.CUdevicePtr });
	driver.mem_advise =                     BuildInFunc("cuMemAdvise",          driver.CUresult, { driver.CUdevicePtr,size_t,driver.CUmem_advise ,driver.CUdevice });
	driver.mem_get_info =                   BuildInFunc("cuMemGetInfo_v2",      driver.CUresult, { size_t->getPointerTo(),size_t->getPointerTo() });
	driver.module_get_function =            BuildInFunc("cuModuleGetFunction",  driver.CUresult, { driver.CUfunction->getPointerTo(),driver.CUmodule,charPtr });
	driver.module_load_data_ex =            BuildInFunc("cuModuleLoadDataEx",   driver.CUresult, { driver.CUmodule->getPointerTo(),voidPtr,int32,driver.CUjit_option,voidPtrPtr });
	driver.launch_kernel =                  BuildInFunc("cuLaunchKernel",       driver.CUresult, { driver.CUfunction,unsign,unsign,unsign,unsign,unsign,unsign,unsign,driver.CUstream,voidPtrPtr,voidPtrPtr });
	driver.stream_synchronize =             BuildInFunc("cuStreamSynchronize",  driver.CUresult, { driver.CUstream });
	driver.event_create =                   BuildInFunc("cuEventCreate",        driver.CUresult, { driver.CUevent,unsign });
	driver.event_record =                   BuildInFunc("cuEventRecord",        driver.CUresult, { driver.CUevent,driver.CUstream });
	driver.event_elapsed_time =             BuildInFunc("cuEventElapsedTime",   driver.CUresult, { floatPtr,driver.CUevent,driver.CUevent });

	BuiltinCudaCheck();
}


