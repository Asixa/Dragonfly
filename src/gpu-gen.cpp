#include "gpu-gen.h"
#include <cstdio>

char* generatePTX(const char* ll, size_t size, const char* filename)
{
	nvvmResult result;
	nvvmProgram program;
	size_t ptx_size;
	char* ptx = nullptr;

	result = nvvmCreateProgram(&program);
	if (result != NVVM_SUCCESS) {
		fprintf(stderr, "nvvmCreateProgram: Failed\n");
		exit(-1);
	}

	result = nvvmAddModuleToProgram(program, ll, size, filename);
	if (result != NVVM_SUCCESS) {
		fprintf(stderr, "nvvmAddModuleToProgram: Failed\n");
		exit(-1);
	}

	result = nvvmCompileProgram(program, 0, NULL);
	if (result != NVVM_SUCCESS) {
		char* msg = nullptr;
		size_t log_size;
		fprintf(stderr, "nvvmCompileProgram: Failed\n");
		nvvmGetProgramLogSize(program, &log_size);
		msg = static_cast<char*>(malloc(log_size));
		nvvmGetProgramLog(program, msg);
		fprintf(stderr, "%s\n", msg);
		free(msg);
		exit(-1);
	}

	result = nvvmGetCompiledResultSize(program, &ptx_size);
	if (result != NVVM_SUCCESS) {
		fprintf(stderr, "nvvmGetCompiledResultSize: Failed\n");
		exit(-1);
	}

	ptx = static_cast<char*>(malloc(ptx_size));
	result = nvvmGetCompiledResult(program, ptx);
	if (result != NVVM_SUCCESS) {
		fprintf(stderr, "nvvmGetCompiledResult: Failed\n");
		free(ptx);
		exit(-1);
	}

	result = nvvmDestroyProgram(&program);
	if (result != NVVM_SUCCESS) {
		fprintf(stderr, "nvvmDestroyProgram: Failed\n");
		free(ptx);
		exit(-1);
	}

	return ptx;
}
