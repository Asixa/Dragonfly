#ifndef TYPE_H
#define TYPE_H
#include <string>

namespace AST {
	struct Type {
	public:
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

		static Type Match();
	};
}
#endif
