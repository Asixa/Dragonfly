#ifndef CPU_CONTEXT_H
#define CPU_CONTEXT_H
#include <string>
#include "LLVM/context.h"
class CudaDriver {
public:
	llvm::GlobalValue* cuda_device,*cuda_module,*cuda_context;
	llvm::Constant* ptx;
	llvm::Function* init,
		* driver_get_version,
		* device_get_count,
		* device_get,
		* device_get_name,
		* device_get_attribute,
		* context_create,
		* context_set_current,
		* context_get_current,
		* stream_create,
		* memcpy_host_to_device,
		* memcpy_device_to_host,
		* memcpy_host_to_device_async,
		* memcpy_device_to_host_async,
		* malloc,
		* malloc_managed,
		* memset,
		* mem_free,
		* mem_advise,
		* mem_get_info,
		* module_get_function,
		* module_load_data_ex,
		* launch_kernel,
		* stream_synchronize,
		* event_create,
		* event_record,
		* event_elapsed_time;
    llvm::Type* CUcontext,
        *CUstream ,
        * CUfunction,
        * CUmodule ,
        * CUevent,
        * CUdevicePtr,
        * CUdevice,
        * CUmem_advise,
        * CUjit_option,
        * CUdevice_attribute,
        * CUresult;
};

class CPUContext :public DFContext {
	void BuiltinCudaCheck();
	void BuiltinAssert();
public:
	CudaDriver driver;
	explicit  CPUContext(std::shared_ptr<AST::Program> program);
	static std::shared_ptr<CPUContext> Create(std::shared_ptr<AST::Program> program);
	void BuiltIn(std::shared_ptr < DFContext> ptr) override;
    
};
#endif
