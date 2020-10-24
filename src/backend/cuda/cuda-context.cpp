#include "cuda-context.h"

#include <llvm/ADT/StringExtras.h>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"
#include "nvvm.h"
#include "cuda.h"

std::string CudaContext::GenPTX() {

	int dev_major=7, dev_minor=0;
	CUdevice device;
	cuDeviceGet(&device, 0);
	cuDeviceComputeCapability(&dev_major, &dev_minor, device);


	std::string module_str;
	llvm::raw_string_ostream str(module_str);
	str << *module;


	nvvmProgram compile_unit;

	// NVVM Initialization
	if (nvvmCreateProgram(&compile_unit)!= NVVM_SUCCESS) {
		llvm::errs() << "libnvvm call failed\n";
	}
	// Create NVVM compilation unit from LLVM IR
	if (nvvmAddModuleToProgram(compile_unit, module_str.c_str(),
		module_str.size(), module->getModuleIdentifier().c_str()) != NVVM_SUCCESS) {
        llvm::errs() << "libnvvm nvvmAddModuleToProgram \n";
	}

	std::string compute_arg = "-arch=compute_";
	compute_arg += llvm::utostr(dev_major);
	compute_arg += llvm::utostr(dev_minor);

	const char* options[] = { compute_arg.c_str() };

	// Compile LLVM IR into PTX
	const auto res = nvvmCompileProgram(compile_unit, 1, options);
	if (res != NVVM_SUCCESS) {
		llvm::errs() << "nvvmCompileProgram failed!\n";
		size_t log_size;
		nvvmGetProgramLogSize(compile_unit, &log_size);
		const auto msg = new char[log_size];
		nvvmGetProgramLog(compile_unit, msg);
		llvm::errs() << msg << "\n";
		delete[] msg;
		return "";
	}

	// Get final PTX size
	size_t ptx_size = 0;
	if (nvvmGetCompiledResultSize(compile_unit, &ptx_size)!=NVVM_SUCCESS) {
		llvm::errs() << "libnvvm GetCompiledResultSize failed\n";
	}

	// Get final PTX
	const auto ptx = new char[ptx_size];
	if (nvvmGetCompiledResult(compile_unit, ptx) != NVVM_SUCCESS) {
		llvm::errs() << "libnvvm GetCompiledResult failed\n";
	}
	// Clean-up libNVVM
	if (nvvmDestroyProgram(&compile_unit) != NVVM_SUCCESS) {
		llvm::errs() << "libnvvm Destroy Program failed\n";
	}

	std::string result(ptx);
	delete[] ptx;
	return result;
}

void CudaContext::Write() {
	DFContext::Write();
	GenPTX();
    // TODO write to file
}

std::shared_ptr<CudaContext> CudaContext::Create(std::shared_ptr<AST::Program> program) {
	auto ptr = std::make_shared<CudaContext>(program);
	contexts.push_back(ptr);
	return ptr;
}

CudaContext::CudaContext(std::shared_ptr<AST::Program> program): DFContext(program) {
    module= std::make_unique<llvm::Module>("nvvm-module", context);
    if (sizeof(void*) == 8) {
		module->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
			"i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
			"v64:64:64-v128:128:128-n16:32:64");
		module->setTargetTriple("nvptx64-nvidia-cuda");
	}
	else {
		module->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
			"i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
			"v64:64:64-v128:128:128-n16:32:64");
		module->setTargetTriple("nvptx-nvidia-cuda");
	}

	GenSample();
 //    llvm::MDNode* kernelMD = llvm::MDNode::get(context, mdVals);
 //    llvm::NamedMDNode* nvvmAnnot = module->getOrInsertNamedMetadata("nvvm.annotations");
	// nvvmAnnot->addOperand(kernelMD);

}

void CudaContext::GenSample() {
    const auto voidTy = llvm::Type::getVoidTy(context);
    const auto floatTy = llvm::Type::getFloatTy(context);
	llvm::Type* i32Ty = llvm::Type::getInt32Ty(context);
	llvm::Type* floatGenericPtrTy = llvm::PointerType::get(floatTy, /* address space */ 0);

    llvm::Type* mandelbrotParamTys[] = { floatGenericPtrTy };
    llvm::FunctionType* mandelbrotTy = llvm::FunctionType::get(voidTy, mandelbrotParamTys,
		false);
    const auto mandelbrotFunc = module->getOrInsertFunction("mandelbrot",mandelbrotTy);

	// Kernel argument types
    llvm::Type* paramTys[] = { floatGenericPtrTy };

	// Kernel function type
    llvm::FunctionType* funcTy = llvm::FunctionType::get(voidTy, paramTys, false);

	// Kernel function
    llvm::Function* func = llvm::Function::Create(funcTy, llvm::GlobalValue::ExternalLinkage,
		"kernel", module.get());
	func->arg_begin()->setName("ptr");

	// 'entry' basic block in kernel function
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", func);

	// Build the entry block
    llvm::IRBuilder<> builder(entry);

	builder.CreateCall(mandelbrotFunc, func->arg_begin());

	builder.CreateRetVoid();

	// Create kernel metadata
 //    llvm::Value* mdVals[] = {
	//   func, llvm::MDString::get(context, "kernel") , llvm::ConstantInt::get(i32Ty, 1)
	// };
	llvm::Metadata* mdVals[] = {
	  llvm::ValueAsMetadata::get(func), llvm::MDString::get(context, "kernel") , llvm::ValueAsMetadata::get(llvm::ConstantInt::get(i32Ty, 1))
	};
	// builder->CreateGlobalStringPtr(llvm::StringRef(frontend::Lexer::MangleStr(value)));
    llvm::MDNode* kernelMD = llvm::MDNode::get(context, mdVals);

    llvm::NamedMDNode* nvvmAnnot = module->getOrInsertNamedMetadata("nvvm.annotations");
	nvvmAnnot->addOperand(kernelMD);
}


