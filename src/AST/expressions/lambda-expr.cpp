#include "AST/expressions/lambda-expr.h"
namespace AST {

    std::shared_ptr<AST::Type> expr::Lambda::Analysis(std::shared_ptr<DFContext>) { return nullptr; }
    llvm::Value* expr::Lambda::Gen(std::shared_ptr<DFContext>, bool is_ptr) { return nullptr; }
}
