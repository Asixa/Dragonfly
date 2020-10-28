#ifndef TYPE_H
#define TYPE_H
#include <string>
#include <vector>
#include <memory>

namespace AST {

    class Type {
	public:

        enum {
            // Basic, Custom, Tuple,Tensor
        };

		int ty = -1;
		int array = -1;
		std::string str = "";


		explicit Type() :array(-1), ty(-1) { }
		explicit Type(const std::string s) :str(s), ty(0), array(-1) { }
		explicit Type(const int t) :ty(t), array(-1) {};
		bool empty() const {
			return ty == -1 || (ty == 0 && str.empty());
		}
		bool operator==(const Type& rhs)
		{
			return ty == rhs.ty && str == rhs.str && array == rhs.array;
		}

		Type& operator=(Type& copy)
			//    ^^^^  Notice the double &&
		{
			ty = copy.ty;
			array = copy.array;
			str = copy.str;
			return *this;
		}

		static std::shared_ptr<AST::Type> Match();
	};

	class  BasicType : public Type {
	public:
        enum BasicTypeDetail{
			BTy_UNSET, BTy_STRING,
            BTy_F8, BTy_F16, BTy_F32, BTy_F64,
			BTy_I1, BTy_I8, BTy_I16, BTy_I32, BTy_I64
		};
		BasicTypeDetail type;
	    static std::shared_ptr<AST::BasicType> Match();
		explicit BasicType(const BasicTypeDetail type) :type(type) {}
		explicit BasicType():type(BTy_UNSET){}
		static std::shared_ptr<AST::Type> string;
		static std::shared_ptr<AST::Type> i32, i64;
		static std::shared_ptr<AST::Type>  f32, f64;
        
    };
	class  CustomType : public Type {
	public:
		static std::shared_ptr<AST::CustomType> Match();
	};
	class  Tuple : public Type {
	public:
		std::vector<std::shared_ptr<AST::Type>> types;
		static std::shared_ptr<AST::Type> Match();
	};
	class  Tensor : public Type {
	public:
		std::shared_ptr < AST::Type> base;
		std::vector<int>shape;
        Tensor(std::shared_ptr<AST::Type> base):base(base){}
		static std::shared_ptr<AST::Tensor> Match(std::shared_ptr<AST::Type> base);
	};
	class  FunctionType : public Type {
	public:
		std::vector<std::shared_ptr<AST::Type>> input;
		std::shared_ptr < AST::Type> returnTy;

		FunctionType(std::shared_ptr<AST::Type> base) :returnTy(base) {}
		static std::shared_ptr<AST::Tensor> Match(std::shared_ptr<AST::Type> base);
	};

}
#endif
