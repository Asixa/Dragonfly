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

	class  BasicType : Type {
		// enum {
		// 	Int8, Custom, Tuple, Tensor
		// };
    };
	class  CustomType : Type {

	};
	class  Tuple : Type {
	public:
		std::vector<Type> types;
		static Tuple Match();
	};
	class  Tensor : Type {
	public:
		Type base;
		std::vector<int>shape;

	};
}
#endif
