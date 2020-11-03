#ifndef TYPE_H
#define TYPE_H
#include <string>
#include <vector>
#include <memory>
#include <llvm/IR/Type.h>
// forward decl
class DFContext;

namespace AST {
    
    namespace decl {class ClassDecl;}


    class Type {
		
	public:
		enum TypeType { Basic, Custom, Tuple, Tensor, Function };
		Type(TypeType ty) :category(ty) {}
		TypeType category;
		// int ty = -1;
		// int array = -1;
		
		// std::string str = "";


		// explicit Type()  { }
		// explicit Type(const std::string s) :str(s), ty(0) { }
		// explicit Type(const int t) :ty(t) {};

		// virtual bool operator==(const Type& rhs)=0;
		//
		// Type& operator=(Type& copy)
		// 	//    ^^^^  Notice the double &&
		// {
		// 	// ty = copy.ty;
		// 	str = copy.str;
		// 	return *this;
		// }

		virtual llvm::Type* ToLLVM(std::shared_ptr<DFContext>) = 0;
		virtual std::string ToString() = 0;
		// virtual std::shared_ptr<Type> Copy() = 0;
		static std::shared_ptr<AST::Type> Match();
	};

	class  BasicType : public Type {
		
		
	public:
		int detail;
		explicit BasicType() :Type(Basic), detail(0) {}
		explicit BasicType(const int detail) :Type(Basic), detail(detail) {}
	    static std::shared_ptr<AST::BasicType> Match();
		static std::shared_ptr<AST::Type> String,Int, Long, Float, Double,Void,Void_Ptr,Boolean;
		llvm::Type* ToLLVM(std::shared_ptr<DFContext>) override;
		std::string ToString()override;
		// std::shared_ptr<Type> Copy()override;
    };
	class  CustomType : public Type {
		std::string str = "";
	public:
		explicit CustomType(const std::shared_ptr<decl::ClassDecl>decl);
		explicit CustomType() :decl(nullptr), Type(Custom) {}
		std::shared_ptr<decl::ClassDecl>decl;
		

		static std::shared_ptr<AST::CustomType> Match();

		llvm::Type* ToLLVM(std::shared_ptr<DFContext>) override;
		// std::shared_ptr<Type> Copy()override;
		std::string ToString()override;

	};
	class  Tuple : public Type {
	public:
		std::vector<std::shared_ptr<AST::Type>> types;

        Tuple():Type(TypeType::Tuple){}
		static std::shared_ptr<AST::Type> Match();
		llvm::Type* ToLLVM(std::shared_ptr<DFContext>) override;
		std::string ToString()override;
	};
	class  Tensor : public Type {
	public:
		std::shared_ptr < AST::Type> base;
		std::vector<int>shape;
        Tensor(std::shared_ptr<AST::Type> base):base(base), Type(TypeType::Tensor) {}
		static std::shared_ptr<AST::Tensor> Match(std::shared_ptr<AST::Type> base);
		llvm::Type* ToLLVM(std::shared_ptr<DFContext>) override;
		std::string ToString()override;
	};
	class  FunctionType : public Type {
	public:
		std::vector<std::shared_ptr<AST::Type>> input;
		std::shared_ptr < AST::Type> returnTy;

		FunctionType(std::shared_ptr<AST::Type> base) :returnTy(base), Type(Function) {}
		static std::shared_ptr<AST::Tensor> Match(std::shared_ptr<AST::Type> base);
		llvm::Type* ToLLVM(std::shared_ptr<DFContext>) override;
		std::string ToString()override;
	};

}
#endif
