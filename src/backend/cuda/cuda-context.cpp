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

	int dev_major=7, dev_minor;
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
