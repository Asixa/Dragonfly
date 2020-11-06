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
#include "AST/Type.h"

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
	const auto this_ptr = std::shared_ptr<CudaContext>(this);
	ast = std::make_shared<frontend::ASTSymbol>(this_ptr);
	llvm = std::make_shared<frontend::LLVMSymbol>(this_ptr);

	BuiltIn(this_ptr);
	GenSample();
 //    llvm::MDNode* kernelMD = llvm::MDNode::get(context, mdVals);
 //    llvm::NamedMDNode* nvvmAnnot = module->getOrInsertNamedMetadata("nvvm.annotations");
	// nvvmAnnot->addOperand(kernelMD);

}
/*******************************************************************
Example annotation from llvm PTX doc:

define void @kernel(float addrspace(1)* %A,
					float addrspace(1)* %B,
					float addrspace(1)* %C);

!nvvm.annotations = !{!0}
!0 = !{void (float addrspace(1)*,
			 float addrspace(1)*,
			 float addrspace(1)*)* @kernel, !"kernel", i32 1}
*******************************************************************/
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
    llvm::Type* paramTys[] = { llvm::Type::getInt32PtrTy(context,1),llvm::Type::getInt32PtrTy(context,1),llvm::Type::getInt32PtrTy(context,1) };

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

	// builder.CreateCall(mandelbrotFunc, func->arg_begin());

	builder.CreateRetVoid();

	MarkAsKernel(func);
}

void CudaContext::BuiltIn(std::shared_ptr < DFContext> ptr) {
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.x", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.y", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.z", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.ntid.x", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.ntid.y", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.ntid.z", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.ctaid.x", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.ctaid.y", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.ctaid.z", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.nctaid.x", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.nctaid.y", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.nctaid.z", AST::BasicType::Int, {});
	BuildInFunc(ptr, "llvm.nvvm.read.ptx.sreg.tid.nctaid.warpsize", AST::BasicType::Int,{});

	
	
	// auto ty=llvm::Type::getInt32PtrTy(context,1);
}

void CudaContext::MarkAsKernel(llvm::Function* func, int dim) {
	InsertNvvmAnnotation(func, "kernel", 1);
    if(dim==0)return;
	InsertNvvmAnnotation(func, "maxntidx", dim);
	InsertNvvmAnnotation(func, "minctasm", 2);
}

void CudaContext::InsertNvvmAnnotation(llvm::Function* f, std::string key, int val) {
	llvm::Metadata* md_args[] = {
	    llvm::ValueAsMetadata::get(f),
	    llvm::MDString::get(context, key),llvm::ValueAsMetadata::get(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), val))
	};
    llvm::MDNode* md_node = llvm::MDNode::get(context, md_args);
	f->getParent() ->getOrInsertNamedMetadata("nvvm.annotations") ->addOperand(md_node);
}


