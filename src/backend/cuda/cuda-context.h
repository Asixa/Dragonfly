



#include "LLVM/context.h"

class CudaContext:public DFContext {

	std::string GenPTX();
    void Write() override;
	static std::shared_ptr<CudaContext> Create(std::shared_ptr<AST::Program> program);
    explicit CudaContext(std::shared_ptr<AST::Program> program):DFContext(program){}
};
