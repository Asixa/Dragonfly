


#ifndef CUDA_CONTEXT_H 
#define CUDA_CONTEXT_H

#include "LLVM/context.h"


class CudaContext:public DFContext {
public:
	// CudaDriver driver;
	std::string GenPTX();
    void Write() override;
	static std::shared_ptr<CudaContext> Create(std::shared_ptr<AST::Program> program);
    explicit CudaContext(std::shared_ptr<AST::Program> program);
	void GenSample();
    void BuiltIn(std::shared_ptr < DFContext> ptr) override;
	void MarkAsKernel(llvm::Function* f,int dim=0);
	void InsertNvvmAnnotation(llvm::Function* f, std::string key, int val);
    
};
#endif
